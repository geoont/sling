// Copyright 2020 Ringgaard Research ApS
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

#include "sling/pyapi/pydatabase.h"

namespace sling {

// Python type declarations.
PyTypeObject PyDatabase::type;
PyMethodTable PyDatabase::methods;
PyMappingMethods PyDatabase::mapping;
PyTypeObject PyCursor::type;

// Check status.
static bool CheckIO(Status status) {
  bool ok = status.ok();
  if (!ok) PyErr_SetString(PyExc_IOError, status.message());
  return ok;
}

void PyDatabase::Define(PyObject *module) {
  InitType(&type, "sling.Database", sizeof(PyDatabase), true);

  type.tp_init = method_cast<initproc>(&PyDatabase::Init);
  type.tp_dealloc = method_cast<destructor>(&PyDatabase::Dealloc);
  type.tp_call = method_cast<ternaryfunc>(&PyDatabase::Start);
  type.tp_iter = method_cast<getiterfunc>(&PyDatabase::Iterator);

  type.tp_as_mapping = &mapping;
  mapping.mp_subscript = method_cast<binaryfunc>(&PyDatabase::Lookup);
  mapping.mp_ass_subscript = method_cast<objobjargproc>(&PyDatabase::Assign);

  methods.Add("close", &PyDatabase::Close);
  methods.AddO("get", &PyDatabase::Get);
  methods.Add("put", &PyDatabase::Put);
  methods.Add("add", &PyDatabase::Add);
  methods.AddO("delete", &PyDatabase::Delete);
  methods.Add("keys", &PyDatabase::Keys);
  methods.Add("values", &PyDatabase::Values);
  methods.Add("items", &PyDatabase::Items);
  methods.Add("position", &PyDatabase::Position);
  type.tp_methods = methods.table();

  RegisterType(&type, module, "Database");

  RegisterEnum(module, "DBOVERWRITE", DBOVERWRITE);
  RegisterEnum(module, "DBADD", DBADD);
  RegisterEnum(module, "DBORDERED", DBORDERED);
  RegisterEnum(module, "DBNEWER", DBNEWER);

  RegisterEnum(module, "DBNEW", DBNEW);
  RegisterEnum(module, "DBUPDATED", DBUPDATED);
  RegisterEnum(module, "DBUNCHANGED", DBUNCHANGED);
  RegisterEnum(module, "DBEXISTS", DBEXISTS);
  RegisterEnum(module, "DBSTALE", DBSTALE);
  RegisterEnum(module, "DBFAULT", DBFAULT);
}

int PyDatabase::Init(PyObject *args, PyObject *kwds) {
  // Initialize defaults.
  this->batchsize = 128;
  this->position = 0;

  // Get arguments.
  static const char *kwlist[] = {"batch", nullptr};
  char *dbname;
  bool ok = PyArg_ParseTupleAndKeywords(
                args, kwds, "s|i", const_cast<char **>(kwlist),
                &dbname, &this->batchsize);
  if (!ok) return -1;

  // Open connection to database.
  db = new DBClient();
  if (!CheckIO(db->Connect(dbname))) return -1;

  return 0;
}

void PyDatabase::Dealloc() {
  delete db;
  Free();
}

PyObject *PyDatabase::Close() {
  if (!CheckIO(db->Close())) return nullptr;
  Py_RETURN_NONE;
}

PyObject *PyDatabase::Get(PyObject *obj) {
  // Get record key.
  Slice key;
  if (!GetData(obj, &key)) return nullptr;

  // Fetch record.
  DBRecord record;
  if (!CheckIO(db->Get(key, &record))) return nullptr;

  // Return tuple with value and version.
  PyObject *value = PyValue(record.value);
  PyObject *version = PyLong_FromLong(record.version);
  PyObject *pair = PyTuple_Pack(2, value, version);
  Py_DECREF(value);
  Py_DECREF(version);
  return pair;
}

PyObject *PyDatabase::Put(PyObject *args, PyObject *kw) {
  // Parse arguments.
  static const char *kwlist[] = {"version", "mode", nullptr};
  DBRecord record;
  PyObject *key = nullptr;
  PyObject *value = nullptr;
  DBMode mode = DBOVERWRITE;
  bool ok = PyArg_ParseTupleAndKeywords(
                args, kw, "OO|li", const_cast<char **>(kwlist),
                &key, &value, &record.version, &mode);
  if (!ok) return nullptr;
  if (!GetData(key, &record.key)) return nullptr;
  if (!GetData(value, &record.value)) return nullptr;

  // Update record in database.
  if (!CheckIO(db->Put(&record, mode))) return nullptr;

  // Return outcome.
  return PyLong_FromLong(record.result);
}

PyObject *PyDatabase::Add(PyObject *args, PyObject *kw) {
  // Parse arguments.
  static const char *kwlist[] = {"version", nullptr};
  DBRecord record;
  PyObject *key = nullptr;
  PyObject *value = nullptr;
  bool ok = PyArg_ParseTupleAndKeywords(
                args, kw, "OO|l", const_cast<char **>(kwlist),
                &key, &value, &record.version);
  if (!ok) return nullptr;
  if (!GetData(key, &record.key)) return nullptr;
  if (!GetData(value, &record.value)) return nullptr;

  // Update record in database.
  if (!CheckIO(db->Add(&record))) return nullptr;

  // Return outcome.
  return PyLong_FromLong(record.result);
}

PyObject *PyDatabase::Delete(PyObject *key) {
  Slice k;
  if (!GetData(key, &k)) return nullptr;
  if (!CheckIO(db->Delete(k))) return nullptr;
  Py_RETURN_NONE;
}

PyObject *PyDatabase::Lookup(PyObject *key) {
  // Get record key.
  DBRecord record;
  if (!GetData(key, &record.key)) return nullptr;

  // Fetch record.
  if (!CheckIO(db->Get(record.key, &record))) return nullptr;

  // Return record value.
  return PyValue(record.value);
}

int PyDatabase::Assign(PyObject *key, PyObject *v) {
  DBRecord record;
  if (!GetData(key, &record.key)) return -1;
  if (!GetData(v, &record.value)) return -1;

  // Update record in database.
  if (!CheckIO(db->Put(&record))) return -1;

  return 0;
}

PyObject *PyDatabase::Iterator() {
  PyCursor *cursor = PyObject_New(PyCursor, &PyCursor::type);
  cursor->Init(this, 0, PyCursor::FULL);
  return cursor->AsObject();
}

PyObject *PyDatabase::Keys() {
  PyCursor *cursor = PyObject_New(PyCursor, &PyCursor::type);
  cursor->Init(this, 0, PyCursor::KEYS);
  return cursor->AsObject();
}

PyObject *PyDatabase::Values() {
  PyCursor *cursor = PyObject_New(PyCursor, &PyCursor::type);
  cursor->Init(this, 0, PyCursor::VALUES);
  return cursor->AsObject();
}

PyObject *PyDatabase::Items() {
  PyCursor *cursor = PyObject_New(PyCursor, &PyCursor::type);
  cursor->Init(this, 0, PyCursor::ITEMS);
  return cursor->AsObject();
}

PyObject *PyDatabase::Start(PyObject *args, PyObject *kw) {
  uint64 start = 0;
  if (!PyArg_ParseTuple(args, "L", &start)) return nullptr;


  PyCursor *cursor = PyObject_New(PyCursor, &PyCursor::type);
  cursor->Init(this, start, PyCursor::FULL);
  return cursor->AsObject();
}

PyObject *PyDatabase::Position() {
  return PyLong_FromLong(position);
}

bool PyDatabase::GetData(PyObject *obj, Slice *data) {
  char *buffer;
  Py_ssize_t length;

  if (PyBytes_Check(obj)) {
    if (PyBytes_AsStringAndSize(obj, &buffer, &length) == -1) return false;
  } else {
    buffer = PyUnicode_AsUTF8AndSize(obj, &length);
    if (buffer == nullptr) return false;
  }

  *data = Slice(buffer, length);
  return true;
}

PyObject *PyDatabase::PyValue(const Slice &slice, bool binary) {
  if (slice.empty()) Py_RETURN_NONE;
  if (!binary) {
    PyObject *str = PyUnicode_FromStringAndSize(slice.data(), slice.size());
    if (str != nullptr) return str;
    PyErr_Clear();
  }
  return PyBytes_FromStringAndSize(slice.data(), slice.size());
}

void PyCursor::Define(PyObject *module) {
  InitType(&type, "sling.Cursor", sizeof(PyCursor), false);
  type.tp_dealloc = method_cast<destructor>(&PyCursor::Dealloc);
  type.tp_iter = method_cast<getiterfunc>(&PyCursor::Self);
  type.tp_iternext = method_cast<iternextfunc>(&PyCursor::Next);
  RegisterType(&type, module, "Cursor");
}

void PyCursor::Init(PyDatabase *pydb, uint64 start, Fields fields) {
  this->pydb = pydb;
  this->fields = fields;
  Py_INCREF(pydb);

  iterator = start;
  next = 0;
  records = new std::vector<DBRecord>;
  buffer = new IOBuffer();
}

void PyCursor::Dealloc() {
  pydb->position = iterator;
  Py_DECREF(pydb);
  delete records;
  delete buffer;
  Free();
}

PyObject *PyCursor::Next() {
  // Fetch next batch of records if needed.
  if (next == records->size()) {
    records->clear();
    Status st = pydb->db->Next(&iterator, pydb->batchsize, records, buffer);
    if (!st.ok()) {
      if (st.code() == ENOENT) {
        PyErr_SetNone(PyExc_StopIteration);
      } else {
        PyErr_SetString(PyExc_IOError, st.message());
      }
      return nullptr;
    }
    next = 0;
  }

  // Return next record in batch.
  DBRecord &record = (*records)[next++];
  switch (fields) {
    case FULL: {
      PyObject *key = PyDatabase::PyValue(record.key, false);
      PyObject *version = PyLong_FromLong(record.version);
      PyObject *value = PyDatabase::PyValue(record.value);
      PyObject *triple = PyTuple_Pack(3, key, version, value);
      Py_DECREF(key);
      Py_DECREF(version);
      Py_DECREF(value);
      return triple;
    }

    case KEYS:
      return PyDatabase::PyValue(record.key, false);

    case VALUES:
      return PyDatabase::PyValue(record.value);

    case ITEMS: {
      PyObject *key = PyDatabase::PyValue(record.key, false);
      PyObject *value = PyDatabase::PyValue(record.value);
      PyObject *pair = PyTuple_Pack(2, key, value);
      Py_DECREF(key);
      Py_DECREF(value);
      return pair;
    }
  }

  return nullptr;
}

PyObject *PyCursor::Self() {
  Py_INCREF(this);
  return AsObject();
}

}  // namespace sling
