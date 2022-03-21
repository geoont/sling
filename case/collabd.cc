// Copyright 2022 Ringgaard Research ApS
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <signal.h>
#include <sys/random.h>
#include <algorithm>
#include <string>
#include <vector>

#include "sling/base/flags.h"
#include "sling/base/init.h"
#include "sling/base/logging.h"
#include "sling/file/file.h"
#include "sling/frame/store.h"
#include "sling/frame/decoder.h"
#include "sling/frame/encoder.h"
#include "sling/net/http-server.h"
#include "sling/net/web-sockets.h"
#include "sling/stream/input.h"
#include "sling/stream/file.h"
#include "sling/stream/memory.h"
#include "sling/util/mutex.h"

using namespace sling;

DEFINE_string(addr, "", "HTTP server address");
DEFINE_int32(port, 7700, "HTTP server port");
DEFINE_int32(workers, 16, "Number of network worker threads");
DEFINE_string(datadir, ".", "Data directory for collaborations");

// Collaboration protocol opcodes.
enum CollabOpcode {
  COLLAB_CREATE  = 1,    // create new collaboration
  COLLAB_DELETE  = 2,    // delete collaboration
  COLLAB_INVITE  = 3,    // invite participant to join collaboration
  COLLAB_JOIN    = 4,    // add user as participant in collaboration
  COLLAB_LOGIN   = 5,    // log-in to collaboration to send and receive updates
  COLLAB_NEWID   = 6,    // new topic id
  COLLAB_UPDATE  = 7,    // update collaboration case

  COLLAB_ERROR   = 127,  // error message
};

// Credential key size.
const int CREDENTIAL_BITS = 128;
const int CREDENTIAL_BYTES = CREDENTIAL_BITS / 8;

// HTTP server.
HTTPServer *httpd = nullptr;

// Commons store with global symbols.
Store *commons;
Names names;
Name n_caseid(names, "caseid");
Name n_main(names, "main");
Name n_topics(names, "topics");
Name n_author(names, "P50");
Name n_participant(names, "P710");

// Termination handler.
void terminate(int signum) {
  VLOG(1) << "Shutdown requested";
  if (httpd != nullptr) httpd->Shutdown();
}

class CollabCase;
class CollabClient;

// Return random key encoded as hex digits.
string RandomKey() {
  uint8 key[CREDENTIAL_BYTES];
  CHECK_EQ(getrandom(key, CREDENTIAL_BYTES, 0), CREDENTIAL_BYTES);
  string str;
  for (int i = 0; i < CREDENTIAL_BYTES; ++i) {
    str.push_back("0123456789abcdef"[key[i] >> 4]);
    str.push_back("0123456789abcdef"[key[i] & 0x0F]);
  }
  return str;
}

// Collaboration protocol packet reader.
class CollabReader {
 public:
  CollabReader(const uint8 *packet, size_t size)
    : stream_(packet, size), input_(&stream_) {}

  // Read varint-encoded integer from packet. Return -1 on error.
  int ReadInt() {
    uint32 value;
    if (!input_.ReadVarint32(&value)) return -1;
    return value;
  }

  // Read variable-size string from packet. Return empty on error.
  string ReadString() {
    string value;
    if (!input_.ReadVarString(&value)) value.clear();
    return value;
  }

  // Read SLING objects from packet.
  Object ReadObjects(Store *store) {
    Decoder decoder(store, &input_, false);
    return decoder.DecodeAll();
  }

 private:
  // Packet input stream.
  ArrayInputStream stream_;

  // Input stream handler.
  Input input_;
};

// Collaboration protocol packet writer.
class CollabWriter {
 public:
  CollabWriter() : output_(&stream_) {}

  // Write varint-encoded integer to packet.
  void WriteInt(int value) {
    output_.WriteVarint32(value);
  }

  // Write variable-size string to packet.
  void WriteString(Text str) {
    output_.WriteVarString(str);
  }

  // Write raw data to output.
  void Write(Slice buffer) {
    output_.Write(buffer.data(), buffer.size());
  }

  // Send packet on websocket.
  void Send(WebSocket *ws) {
    output_.Flush();
    Slice packet = stream_.data();
    ws->Send(packet.data(), packet.size());
  }

  Output *output() { return &output_; }

 private:
  // Packet output stream.
  ArrayOutputStream stream_;

  // Output stream handler.
  Output output_;
};

// A collaboration case is a shared case managed by the collaboration server.
class CollabCase {
 public:
  CollabCase() : store_(commons) {}
  CollabCase(int caseid) : store_(commons), caseid_(caseid) {}

  // Read case file from input packet.
  bool Parse(CollabReader *reader) {
    MutexLock lock(&mu_);
    casefile_ = reader->ReadObjects(&store_).AsFrame();
    if (casefile_.IsNil()) return false;

    // Get case id.
    caseid_ = casefile_.GetInt(n_caseid);
    if (caseid_ == 0) return false;

    // Get main author for case.
    Frame main = casefile_.GetFrame(n_main);
    if (!main.valid()) return false;
    author_ = main.GetHandle(n_author);
    if (author_.IsNil()) return false;

    return true;
  }

  // Encode case to output packet.
  void EncodeCase(CollabWriter *writer) {
    MutexLock lock(&mu_);
    Encoder encoder(&store_, writer->output(), false);
    Serialize(&encoder);
  }

  // Return case id.
  int caseid() const { return caseid_; }

  // Return case author.
  Handle author() const { return author_; }

  // Get main author id for case.
  Text Author() const {
    MutexLock lock(&mu_);
    return store_.FrameId(author_);
  }

  // Add participant.
  void AddParticipant(const string &id, const string &credentials) {
    MutexLock lock(&mu_);
    participants_.emplace_back(id, credentials);
  }

  // Login user.
  bool Login(CollabClient *client,
             const string &id,
             const string &credentials) {
    MutexLock lock(&mu_);

    // Check user access.
    bool valid = false;
    for (const User &user : participants_) {
      if (user.id == id && user.credentials == credentials) {
        valid = true;
        break;
      }
    }
    if (!valid) return false;

    // Add client as listener.
    clients_.push_back(client);
    return true;
  }

  // Logout user.
  void Logout(CollabClient *client) {
    MutexLock lock(&mu_);
    auto it = std::find(clients_.begin(), clients_.end(), client);
    if (it != clients_.end()) {
      clients_.erase(it);
    }
  }

  // Read case from file.
  bool ReadCase() {
    MutexLock lock(&mu_);

    // Open case file.
    File *f;
    Status st = File::Open(CaseFileName(caseid_), "r", &f);
    if (!st.ok()) {
      LOG(ERROR) << "Error opening case# " << caseid() << ": " << st;
      return false;
    }

    // Decode case.
    FileInputStream stream(f);
    Input input(&stream);
    Decoder decoder(&store_, &input);
    casefile_ = decoder.DecodeAll().AsFrame();
    if (casefile_.IsNil()) return false;

    // Get main author for case.
    Frame main = casefile_.GetFrame(n_main);
    if (!main.valid()) return false;
    author_ = main.GetHandle(n_author);
    if (author_.IsNil()) return false;

    return true;
  }

  // Read participants from file.
  bool ReadParticipants() {
    MutexLock lock(&mu_);

    // Read user file.
    string content;
    Status st = File::ReadContents(UserFileName(caseid_), &content);
    if (!st) return false;

    // Parse users.
    for (Text &line : Text(content).split('\n')) {
      auto fields = line.split(' ');
      CHECK_EQ(fields.size(), 2);
      auto id = fields[0].trim();
      auto credentials = fields[1].trim();
      participants_.emplace_back(id.str(), credentials.str());
    }

    return true;
  }

  // Write case to file.
  void WriteCase() {
    MutexLock lock(&mu_);
    FileOutputStream stream(CaseFileName(caseid_));
    Output output(&stream);
    Encoder encoder(&store_, &output);
    Serialize(&encoder);
  }

  // Write participants to file.
  void WriteParticipants() {
    MutexLock lock(&mu_);
    File *f = File::OpenOrDie(UserFileName(caseid_), "w");
    for (const User &user : participants_) {
      f->WriteLine(user.id + " " + user.credentials);
    }
    f->Close();
  }

  // Check for existing case.
  static bool Exists(int caseid) {
    return File::Exists(CaseFileName(caseid));
  }

 private:
  // Return case filename.
  static string CaseFileName(int caseid) {
    return FLAGS_datadir + "/" + std::to_string(caseid) + ".sling";
  }

  // Return case filename.
  static string UserFileName(int caseid) {
    return FLAGS_datadir + "/" + std::to_string(caseid) + ".access";
  }

  // Serialize collaboration case.
  void Serialize(Encoder *encoder) {
    Array topics = casefile_.Get(n_topics).AsArray();
    if (topics.valid()) {
      for (int i = 0; i < topics.length(); ++i) {
        encoder->Encode(topics.get(i));
      }
    }
    encoder->Encode(casefile_);
  }

  // Case store for collaboration.
  Store store_;

  // Case file.
  Frame casefile_;

  // Case id.
  int caseid_ = 0;

  // Case author.
  Handle author_ = Handle::nil();

  // User id and credentials.
  struct User {
    User(const string &id, const string &credentials)
      : id(id), credentials(credentials) {}
    string id;
    string credentials;
  };

  // Users currently connected to collaboration.
  std::vector<CollabClient *> clients_;

  // Participants in collaboration.
  std::vector<User> participants_;

  // Users invited as participants in collaboration.
  std::vector<User> invites_;

  // Mutex for serializing access.
  mutable Mutex mu_;
};

// A collaboration service manages a number of collaboration cases with
// clients updating and monitoring live changes.
class CollabService {
 public:
  // Register collaboration service in HTTP server.
  void Register(HTTPServer *http) {
    http->Register("/collab", this, &CollabService::Process);
  }

  // Process HTTP websocket requests.
  void Process(HTTPRequest *request, HTTPResponse *response);

  // Add case to collaboration.
  void Add(CollabCase *collab) {
    MutexLock lock(&mu_);
    collaborations_.push_back(collab);
  }

  // Find case.
  CollabCase *FindCase(int caseid) {
    MutexLock lock(&mu_);

    // Try to find case that has already been loaded.
    for (auto *collab : collaborations_) {
      if (collab->caseid() == caseid) return collab;
    }

    // Try to load case from file.
    LOG(INFO) << "Loading case #" << caseid;
    CollabCase *collab = new CollabCase(caseid);
    if (!collab->ReadCase() || !collab->ReadParticipants()) {
      delete collab;
      return nullptr;
    }

    // Add collaboration.
    collaborations_.push_back(collab);
    return collab;
  }


 private:
  // Active collaboration cases.
  std::vector<CollabCase *> collaborations_;

  // Mutex for serializing access.
  Mutex mu_;
};

// A collaboration client is a participant in a collaboration.
class CollabClient : public WebSocket {
 public:
  CollabClient(CollabService *service, SocketConnection *conn)
    : WebSocket(conn),
      service_(service) {}

  ~CollabClient() {
    if (collab_) {
      LOG(INFO) << "Logout user " << userid_
                << " from case #" << collab_->caseid();
      collab_->Logout(this);
    }
  }

  void Receive(const uint8 *data, uint64 size, bool binary) override {
    CollabReader reader(data, size);
    int op = reader.ReadInt();
    switch (op) {
      case COLLAB_CREATE: Create(&reader); break;
      case COLLAB_LOGIN: Login(&reader); break;
      default:
        LOG(ERROR) << "Invalid collab op: " << op;
    }
  }

  // Create new collaboration.
  void Create(CollabReader *reader) {
    // Make sure client is not already connected to collaboration.
    if (collab_ != nullptr) {
      Error("already connected to a collaboration");
      return;
    }

    // Receive initial case for collaboration.
    CollabCase *collab = new CollabCase();
    if (!collab->Parse(reader)) {
      Error("invalid case format");
      delete collab;
      return;
    }

    // Make sure case is not already registered.
    if (CollabCase::Exists(collab->caseid())) {
      Error("case is already registered as a collaboration");
      delete collab;
      return;
    }

    // Add user as participant in collaboration.
    string userid = collab->Author().str();
    if (userid.find(' ') != -1) {
      Error("invalid user id");
      delete collab;
      return;
    }
    string credentials = RandomKey();
    collab->AddParticipant(userid, credentials);

    // Add collaboration to service.
    service_->Add(collab);

    // Flush to disk.
    collab->WriteCase();
    collab->WriteParticipants();

   // Return reply with signals to the client that the collaboration server
   // has taken ownership of the case.
   CollabWriter writer;
   writer.WriteInt(COLLAB_CREATE);
   writer.WriteString(credentials);
   writer.Send(this);

    LOG(INFO) << "Created new collaboration for case #" << collab->caseid()
              << " author " << collab->Author();
  }

  // Log-in user to collaboration.
  void Login(CollabReader *reader) {
    // Make sure client is not already connected to collaboration.
    if (collab_ != nullptr) {
      Error("already connected to a collaboration");
      return;
    }

    // Receive <caseid> <user> <credentials>.
    int caseid = reader->ReadInt();
    string userid = reader->ReadString();
    string credentials = reader->ReadString();
    LOG(INFO) << "Login " << userid << " to case #" << caseid;

    // Get case.
    collab_ = service_->FindCase(caseid);
    if (collab_ == nullptr) {
      Error("Unknown collaboration");
      return;
    }

    // Log into collaboration to send and receive updates.
    if (!collab_->Login(this, userid, credentials)) {
      LOG(WARNING) << "Access to case #" << caseid << " denied for " << userid;
      Error("access denied");
      collab_ = nullptr;
      return;
    }
    userid_ = userid;

    // Return case.
     CollabWriter writer;
     writer.WriteInt(COLLAB_LOGIN);
     collab_->EncodeCase(&writer);
     writer.Send(this);
  }

  // Return error message to client.
  void Error(const char *message) {
    CollabWriter writer;
    writer.WriteInt(COLLAB_ERROR);
    writer.WriteString(message);
    writer.Send(this);
  }

 private:
  // Collaboration service.
  CollabService *service_;

  // Current collaboration for client.
  CollabCase *collab_ = nullptr;

  // Collaboration user id.
  string userid_;
};

void CollabService::Process(HTTPRequest *request, HTTPResponse *response) {
  CollabClient *client = new CollabClient(this, request->conn());
  if (!WebSocket::Upgrade(client, request, response)) {
    delete client;
    response->SendError(404);
    return;
  }
}

int main(int argc, char *argv[]) {
  InitProgram(&argc, &argv);

  // Initialize commons store.
  commons = new Store();
  names.Bind(commons);
  commons->Freeze();

  // Initialize collaboration service.
  CollabService *collabd = new CollabService();

  // Install signal handlers to handle termination.
  signal(SIGTERM, terminate);
  signal(SIGINT, terminate);

  // Start HTTP server.
  LOG(INFO) << "Start HTTP server on port " << FLAGS_port;
  SocketServerOptions sockopts;
  sockopts.num_workers = FLAGS_workers;
  httpd = new HTTPServer(sockopts, FLAGS_addr.c_str(), FLAGS_port);
  collabd->Register(httpd);
  CHECK(httpd->Start());
  LOG(INFO) << "Collaboration server running";
  httpd->Wait();

  // Shut down.
  LOG(INFO) << "Shutting down HTTP server";
  delete httpd;
  httpd = nullptr;

  LOG(INFO) << "Shutting down collaboration service";
  delete collabd;
  collabd = nullptr;

  LOG(INFO) << "Done";
  return 0;
}

