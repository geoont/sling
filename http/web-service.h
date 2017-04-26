#ifndef HTTP_WEB_SERVICE_H_
#define HTTP_WEB_SERVICE_H_

#include <string>

#include "base/types.h"
#include "frame/object.h"
#include "http/http-server.h"
#include "string/text.h"

namespace sling {

// Web service handler.
class WebService {
 public:
  enum Format {
    EMPTY,    // empty request or response
    UNKNOWN,  // unknown request or response format; check content type
    ENCODED,  // frames in binary encoding (application/sling)
    TEXT,     // frames in text format (text/sling)
    COMPACT,  // frames in compact text format, i.e. no indentation
    JSON,     // human-readable JSON encoding with indentation (text/json)
    CJSON,    // compact JSON (application/json)
  };

  // Initialize web service from HTTP request and response.
  WebService(Store *commons, HTTPRequest *request, HTTPResponse *response);

  // Generate response.
  ~WebService();

  // Get URL query parameters.
  Text Get(Text name) const;
  int Get(Text name, int defval);
  bool Get(Text name, bool defval);

  // Parsed input and output.
  const Object &input() const { return input_; }
  void set_input(const Object &input) { input_ = input; }
  const Object &output() const { return output_; }
  void set_output(const Object &output) { output_ = output; }

  // Return request and response objects.
  HTTPRequest *request() const { return request_; }
  HTTPResponse *response() const { return response_; }

  // Store for input and output.
  Store *store() { return &store_; }

  // Input and output format.
  Format input_format() const { return input_format_; }
  Format output_format() const { return output_format_; }
  void set_output_format(Format output_format) {
    output_format_ = output_format;
  }

  bool byref() const { return byref_; }
  void set_byref(bool byref) { byref_ = byref; }

 private:
  // URL query parameter.
  struct Parameter {
    Parameter(const string &n, const string &v) : name(n), value(v) {}
    string name;
    string value;
  };

  // Store for request and response.
  Store store_;

  // HTTP request and response.
  HTTPRequest *request_;
  HTTPResponse *response_;

  // URL query parameters.
  std::vector<Parameter> parameters_;

  // Parsed input and output.
  Object input_;
  Object output_;

  // Input and output format.
  Format input_format_ = EMPTY;
  Format output_format_ = EMPTY;

  // Allow references.
  bool byref_ = false;
};

}  // namespace sling

#endif  // HTTP_WEB_SERVICE_H_

