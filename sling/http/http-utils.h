// Copyright 2017 Google Inc.
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

#ifndef SLING_HTTP_HTTP_UTILS_H_
#define SLING_HTTP_HTTP_UTILS_H_

#include <string.h>
#include <string>
#include <vector>

#include "sling/base/types.h"
#include "sling/string/text.h"

namespace sling {

// HTTP memory buffer.
struct HTTPBuffer {
 public:
  ~HTTPBuffer() { free(floor); }

  // Buffer size.
  int size() const { return end - start; }

  // Buffer capacity.
  int capacity() const { return ceil - floor; }

  // Number of bytes left in buffer.
  int remaining() const { return ceil - end; }

  // Whether buffer is empty.
  bool empty() const { return start == end; }

  // Whether buffer is full.
  bool full() const { return end == ceil; }

  // Clear buffer and allocate space.
  void reset(int size);

  // Flush buffer by moving the used part to the beginning of the buffer.
  void flush();

  // Make room in buffer.
  void ensure(int minfree);

  // Clear buffer;
  void clear();

  // Get next line from buffer and nul terminate it. Returns null if no newline
  // is found. White space and HTTP header continuations are replaced with
  // spaces and trailing whitespace is removed.
  char *gets();

  // Append string to buffer.
  void append(const char *data, int size);
  void append(const char *str) { if (str) append(str, strlen(str)); }

  char *floor = nullptr;  // start of allocated memory
  char *ceil = nullptr;   // end of allocated memory
  char *start = nullptr;  // start of used part of buffer
  char *end = nullptr;    // end of used part of buffer
};

// HTTP header.
struct HTTPHeader {
  HTTPHeader(char *n, char *v) : name(n), value(v) {}
  char *name;
  char *value;
};

// URL query string parser.
class URLQuery {
 public:
  // Parse URL query string.
  URLQuery(const char *query);

  // Get URL query parameter.
  Text Get(Text name) const;
  int Get(Text name, int defval) const;
  bool Get(Text name, bool defval) const;

 private:
  // URL query parameter.
  struct Parameter {
    Parameter(const string &n, const string &v) : name(n), value(v) {}
    string name;
    string value;
  };

  // URL query parameters.
  std::vector<Parameter> parameters_;
};

// HTTP methods.
enum HTTPMethod {
  HTTP_GET     = 0,
  HTTP_HEAD    = 1,
  HTTP_POST    = 2,
  HTTP_PUT     = 3,
  HTTP_DELETE  = 4,
  HTTP_CONNECT = 5,
  HTTP_OPTIONS = 6,
  HTTP_TRACE   = 7,
  HTTP_PATCH   = 8,
  HTTP_INVALID = -1,
};

// Decode HTTP method name.
HTTPMethod GetHTTPMethod(const char *name);

// Decode URL component and append to output.
bool DecodeURLComponent(const char *url, int length, string *output);
bool DecodeURLComponent(const char *url, string *output);

// Escape text for HTML.
string HTMLEscape(const char *text, int size);

inline string HTMLEscape(const char *text) {
  return HTMLEscape(text, strlen(text));
}

inline string HTMLEscape(const string &text) {
  return HTMLEscape(text.data(), text.size());
}

// Convert time to RFC date format.
static const int RFCTIME_SIZE = 32;
char *RFCTime(time_t t, char *buf);

// Parse RFC date as time stamp.
time_t ParseRFCTime(const char *timestr);

}  // namespace sling

#endif  // SLING_HTTP_HTTP_UTILS_H_

