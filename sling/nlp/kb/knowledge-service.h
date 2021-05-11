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

#ifndef NLP_KB_KNOWLEDGE_SERVICE_H_
#define NLP_KB_KNOWLEDGE_SERVICE_H_

#include <string>

#include "sling/base/types.h"
#include "sling/frame/object.h"
#include "sling/frame/store.h"
#include "sling/net/http-server.h"
#include "sling/net/static-content.h"
#include "sling/nlp/document/document.h"
#include "sling/nlp/document/document-tokenizer.h"
#include "sling/nlp/document/lex.h"
#include "sling/nlp/kb/calendar.h"
#include "sling/nlp/kb/name-table.h"
#include "sling/nlp/kb/xref.h"

namespace sling {
namespace nlp {

class KnowledgeService {
 public:
  // Information collected for item.
  struct Item {
    Item(Store *store)
        : properties(store),
          xrefs(store),
          categories(store),
          gallery(store) {}

    Handles properties;
    Handles xrefs;
    Handles categories;
    Handles gallery;
  };

  ~KnowledgeService() {
    if (docnames_) docnames_->Release();
  }

  // Load and initialize knowledge base.
  void Load(Store *kb, const string &name_table);

  // Load cross-reference table.
  void LoadXref(const string &xref_table);

  // Register knowledge base service.
  void Register(HTTPServer *http);

  // Handle KB name queries.
  void HandleQuery(HTTPRequest *request, HTTPResponse *response);

  // Handle KB item requests.
  void HandleGetItem(HTTPRequest *request, HTTPResponse *response);

  // Handle KB frame requests.
  void HandleGetFrame(HTTPRequest *request, HTTPResponse *response);

 private:
  // Fetch properties.
  void FetchProperties(const Frame &item, Item *info);

  // Get standard properties (ref, name, and description).
  void GetStandardProperties(const Frame &item, Builder *builder) const;

  // Sort items in cronological order.
  void SortChronologically(Store *store, Handles *values) const;

  // Get canonical date for frame.
  Date GetCanonicalDate(const Frame &frame) const;

  // Get canonical order for frame.
  int64 GetCanonicalOrder(const Frame &frame) const;

  // Get unit name.
  string UnitName(const Frame &unit);

  // Convert value to readable text.
  string AsText(Handle value);

  // Property information.
  struct Property {
    Handle id;
    Handle name;
    Handle datatype;
    string url;
    bool image;
    int order = kint32max;
  };

  // Property name and id for sorting xref properties.
  struct PropName {
    PropName(Text name, Handle id) : name(name), id(id) {}

    // Sort predicate for case-insensitive ordering.
    bool operator<(const PropName &other) const {
      return name.casecompare(other.name) < 0;
    }

    Text name;
    Handle id;
  };

  // Knowledge base store.
  Store *kb_ = nullptr;

  // Property map.
  HandleMap<Property> properties_;

  // Calendar.
  Calendar calendar_;

  // Name table.
  NameTable aliases_;

  // Identifier cross-reference.
  XRefMapping xref_;

  // Knowledge base browser app.
  StaticContent common_{"/common", "app"};
  StaticContent app_{"/kb", "sling/nlp/kb/app"};

  // Document tokenizer and lexer.
  DocumentTokenizer tokenizer_;
  DocumentLexer lexer_{&tokenizer_};

  // Symbols.
  Names names_;
  DocumentNames *docnames_ = nullptr;
  Name n_name_{names_, "name"};
  Name n_description_{names_, "description"};
  Name n_media_{names_, "media"};
  Name n_role_{names_, "role"};
  Name n_target_{names_, "target"};
  Name n_properties_{names_, "properties"};
  Name n_qualifiers_{names_, "qualifiers"};
  Name n_xrefs_{names_, "xrefs"};
  Name n_property_{names_, "property"};
  Name n_values_{names_, "values"};
  Name n_categories_{names_, "categories"};
  Name n_gallery_{names_, "gallery"};
  Name n_type_{names_, "type"};
  Name n_text_{names_, "text"};
  Name n_ref_{names_, "ref"};
  Name n_url_{names_, "url"};
  Name n_lex_{names_, "lex"};
  Name n_document_{names_, "document"};
  Name n_matches_{names_, "matches"};
  Name n_lang_{names_, "lang"};
  Name n_nsfw_{names_, "nsfw"};

  Name n_xref_type_{names_, "/w/xref"};
  Name n_item_type_{names_, "/w/item"};
  Name n_property_type_{names_, "/w/property"};
  Name n_url_type_{names_, "/w/url"};
  Name n_text_type_{names_, "/w/text"};
  Name n_quantity_type_{names_, "/w/quantity"};
  Name n_geo_type_{names_, "/w/geo"};
  Name n_media_type_{names_, "/w/media"};
  Name n_time_type_{names_, "/w/time"};
  Name n_string_type_{names_, "/w/string"};
  Name n_lat_{names_, "/w/lat"};
  Name n_lng_{names_, "/w/lng"};
  Name n_amount_{names_, "/w/amount"};
  Name n_unit_{names_, "/w/unit"};
  Name n_category_{names_, "/w/item/category"};

  Name n_instance_of_{names_, "P31"};
  Name n_formatter_url_{names_, "P1630"};
  Name n_representative_image_{names_, "Q26940804"};
  Name n_image_{names_, "P18"};
  Name n_inverse_label_item_{names_, "P7087"};
  Name n_reason_for_deprecation_{names_, "P2241"};
  Name n_applies_if_regex_matches_{names_, "P8460"};

  Name n_unit_symbol_{names_, "P558"};
  Name n_writing_system_{names_, "P282"};
  Name n_latin_script_{names_, "Q8229"};
  Name n_language_{names_, "P2439"};
  Name n_name_language_{names_, "P407"};

  Name n_start_time_{names_, "P580"};
  Name n_end_time_{names_, "P582"};
  Name n_point_in_time_{names_, "P585"};
  Name n_series_ordinal_{names_, "P1545"};
  Name n_media_legend_{names_, "P2096"};
  Name n_has_quality_{names_, "P1552"};
  Name n_statement_subject_of_{names_, "P805"};
  Name n_not_safe_for_work_{names_, "Q2716583"};
};

}  // namespace nlp
}  // namespace sling

#endif  // NLP_KB_KNOWLEDGE_SERVICE_H_

