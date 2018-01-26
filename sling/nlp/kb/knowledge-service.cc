#include "sling/nlp/kb/knowledge-service.h"

#include <math.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "sling/base/logging.h"
#include "sling/base/types.h"
#include "sling/frame/object.h"
#include "sling/frame/serialization.h"
#include "sling/frame/store.h"
#include "sling/http/http-server.h"
#include "sling/http/web-service.h"
#include "sling/nlp/kb/calendar.h"
#include "sling/string/text.h"
#include "sling/string/strcat.h"

namespace sling {
namespace nlp {

// Convert geo coordinate from decimal to minutes and seconds.
static string ConvertGeoCoord(double coord, bool latitude) {
  // Compute direction.
  const char *sign;
  if (coord < 0) {
    coord = -coord;
    sign = latitude ? "S" : "W";
  } else {
    sign = latitude ? "N" : "E";
  }

  // Compute degrees.
  double integer;
  double remainder = modf(coord, &integer);
  int degrees = static_cast<int>(integer);

  // Compute minutes.
  remainder = modf(remainder * 60, &integer);
  int minutes = static_cast<int>(integer);

  // Compute seconds.
  remainder = modf(remainder * 60, &integer);
  int seconds = static_cast<int>(integer + 0.5);

  // Build coordinate string.
  return StrCat(degrees, "°", minutes, "'", seconds, "\"", sign);
}

void KnowledgeService::Load(const string &knowledge_base,
                            const string &name_table) {
  // Load knowledge base into the commons store.
  LOG(INFO) << "Loading knowledge base from " << knowledge_base;
  LoadStore(knowledge_base, &kb_);

  // Bind names and freeze store.
  CHECK(names_.Bind(&kb_));
  kb_.Freeze();

  // Get meta data for properties.
  for (const Slot &s : Frame(&kb_, kb_.Lookup("/w/properties"))) {
    if (s.name == Handle::id()) continue;
    Frame property(&kb_, s.value);
    Property p;

    // Get property id and name.
    p.id = s.name;
    p.name = property.GetHandle(n_name_);

    // Property data type.
    p.datatype = property.GetHandle(n_datatype_);

    // Get URL formatter for property.
    Handle formatter = property.Resolve(n_formatter_url_);
    if (kb_.IsString(formatter)) {
      p.url = String(&kb_, formatter).value();
    }

    // Check if property is a representative image for the item.
    p.image = false;
    p.alternate_image = false;
    for (const Slot &ps : property) {
      if (ps.name == n_instance_of_ && ps.value == n_representative_image_) {
        if (property == n_image_) {
          p.image = true;
        } else {
          p.alternate_image = true;
        }
      }
    }

    // Add property.
    properties_[p.id] = p;
  }

  // Initialize calendar.
  calendar_.Init(&kb_);

  // Load name table.
  LOG(INFO) << "Loading name table from " << name_table;
  aliases_.Load(name_table);
}

void KnowledgeService::Register(HTTPServer *http) {
  http->Register("/kb/query", this, &KnowledgeService::HandleQuery);
  http->Register("/kb/item", this, &KnowledgeService::HandleGetItem);
  app_.Register(http);
}

void KnowledgeService::HandleQuery(HTTPRequest *request,
                                   HTTPResponse *response) {
  WebService ws(&kb_, request, response);

  // Get query
  Text query = ws.Get("q");
  int window = ws.Get("window", 5000);
  int limit = ws.Get("limit", 30);
  int boost = ws.Get("boost", 1000);
  LOG(INFO) << "Name query: " << query;

  // Lookup name in name table.
  std::vector<Text> matches;
  if (!query.empty()) {
    aliases_.LookupPrefix(query, window, boost, &matches);
  }

  // Check for exact match with id.
  Handles results(ws.store());
  Handle idmatch = kb_.Lookup(query);
  if (!idmatch.IsNil()) {
    Frame item(&kb_, idmatch);
    if (item.valid()) {
      Builder match(ws.store());
      GetStandardProperties(item, &match);
      results.push_back(match.Create().handle());
    }
  }

  // Generate response.
  Builder b(ws.store());
  for (Text id : matches) {
    if (results.size() >= limit) break;
    Frame item(&kb_, kb_.Lookup(id));
    if (item.invalid()) continue;
    Builder match(ws.store());
    GetStandardProperties(item, &match);
    results.push_back(match.Create().handle());
  }
  b.Add(n_matches_,  Array(ws.store(), results));

  // Return response.
  ws.set_output(b.Create());
}

void KnowledgeService::HandleGetItem(HTTPRequest *request,
                                     HTTPResponse *response) {
  WebService ws(&kb_, request, response);

  // Look up item in knowledge base.
  Text itemid = ws.Get("id");
  LOG(INFO) << "Look up item '" << itemid << "'";
  Handle handle = kb_.LookupExisting(itemid);
  if (handle.IsNil()) {
    response->SendError(404, nullptr, "Item not found");
    return;
  }

  // Generate response.
  Frame item(ws.store(), handle);
  if (!item.valid()) {
    response->SendError(404, nullptr, "Invalid item");
    return;
  }
  Builder b(ws.store());
  GetStandardProperties(item, &b);
  Handle dt = item.GetHandle(n_datatype_);
  if (!dt.IsNil()) b.Add(n_type_, dt);

  // Fetch properties.
  Item info(ws.store());
  FetchProperties(item, &info);
  b.Add(n_properties_, Array(ws.store(), info.properties));
  b.Add(n_xrefs_, Array(ws.store(), info.xrefs));

  // Set item image.
  if (!info.image.IsNil()) {
    b.Add(n_thumbnail_, info.image);
  } else if (!info.alternate_image.IsNil()) {
    b.Add(n_thumbnail_, info.alternate_image);
  }

  // Return response.
  ws.set_output(b.Create());
}

void KnowledgeService::FetchProperties(const Frame &item, Item *info) {
  // Collect properties and values.
  HandleMap<Handles *> property_map;
  for (const Slot &s : item) {
    // Skip non-property slots.
    if (properties_.find(s.name) == properties_.end()) continue;

    // Get property list for property.
    Handles *property_list = nullptr;
    auto f = property_map.find(s.name);
    if (f != property_map.end()) {
      property_list = f->second;
    } else {
      property_list = new Handles(item.store());
      property_map[s.name] = property_list;
    }

    // Add property value.
    property_list->push_back(s.value);
  }

  // Build property lists.
  for (auto it : property_map) {
    const auto f = properties_.find(it.first);
    CHECK(f != properties_.end());
    const Property &property = f->second;

    // Add property information.
    Builder p(item.store());
    p.Add(n_property_, property.name);
    p.Add(n_ref_, property.id);
    p.Add(n_type_, property.datatype);

    // Add property values.
    Handles values(item.store());
    for (Handle h : *it.second) {
      // Resolve value.
      Handle value = h;
      bool qualified = false;
      if (kb_.IsFrame(h)) {
        Handle qua = Frame(&kb_, h).GetHandle(Handle::is());
        if (!qua.IsNil()) {
          value = qua;
          qualified = true;
        }
      }

      // Add property value based on property type.
      Builder v(item.store());
      if (property.datatype == n_item_type_) {
        // Add reference to other item.
        Frame ref(&kb_, value);
        if (ref.valid()) {
          GetStandardProperties(ref, &v);
        }
      } else if (property.datatype == n_xref_type_) {
        // Add external reference.
        String identifier(&kb_, value);
        v.Add(n_text_, identifier);
      } else if (property.datatype == n_property_type_) {
        // Add reference to property.
        Frame ref(&kb_, value);
        if (ref.valid()) {
          GetStandardProperties(ref, &v);
        }
      } else if (property.datatype == n_string_type_) {
        // Add string value.
        v.Add(n_text_, value);
      } else if (property.datatype == n_text_type_) {
        // Add text value.
        v.Add(n_text_, value);
      } else if (property.datatype == n_url_type_) {
        // Add URL value.
        v.Add(n_text_, value);
        v.Add(n_url_, value);
      } else if (property.datatype == n_media_type_) {
        // Add image.
        v.Add(n_text_, value);

        // Set representative image for item.
        if (property.image && info->image.IsNil()) {
          info->image = value;
        }
        if (property.alternate_image && info->alternate_image.IsNil()) {
          info->alternate_image = value;
        }
      } else if (property.datatype == n_coord_type_) {
        // Add coordinate value.
        Frame coord(&kb_, value);
        double lat = coord.GetFloat(n_lat_);
        double lng = coord.GetFloat(n_lng_);
        v.Add(n_text_, StrCat(ConvertGeoCoord(lat, true), ", ",
                              ConvertGeoCoord(lng, false)));
        v.Add(n_url_, StrCat("http://maps.google.com/maps?q=",
                              lat, ",", lng));
      } else if (property.datatype == n_quantity_type_) {
        // Add quantity value.
        string text = ToText(&kb_, kb_.Resolve(value));
        if (kb_.IsFrame(value)) {
          Frame quantity(&kb_, value);
          Frame unit = quantity.GetFrame(n_unit_);
          if (unit.valid()) {
            text.append(" ");
            if (unit.Has(n_unit_symbol_)) {
              Handle unit_symbol = unit.Resolve(n_unit_symbol_);
              if (kb_.IsString(unit_symbol)) {
                text.append(String(&kb_, unit_symbol).value());
              }
            } else {
              text.append(unit.GetString(n_name_));
            }
          }
        }
        v.Add(n_text_, text);
      } else if (property.datatype == n_time_type_) {
        // Add time value.
        Object time(&kb_, value);
        v.Add(n_text_, calendar_.DateAsString(time));
      }

      // Add URL if property has URL formatter.
      if (!property.url.empty() && kb_.IsString(value)) {
        String identifier(&kb_, value);
        string url = property.url;
        int pos = url.find("$1");
        if (pos != -1) {
          Text replacement = identifier.text();
          url.replace(pos, 2, replacement.data(), replacement.size());
        }
        if (!url.empty()) v.Add(n_url_, url);
      }

      // Get qualifiers.
      if (qualified) {
        Item qualifiers(item.store());
        FetchProperties(Frame(item.store(), h), &qualifiers);
        if (!qualifiers.properties.empty()) {
          v.Add(n_qualifiers_, Array(item.store(), qualifiers.properties));
        }
      }

      values.push_back(v.Create().handle());
    }
    p.Add(n_values_, Array(item.store(), values));

    // Add property to property list.
    if (property.datatype == n_xref_type_) {
      info->xrefs.push_back(p.Create().handle());
    } else {
      info->properties.push_back(p.Create().handle());
    }
    delete it.second;
  }
}

void KnowledgeService::GetStandardProperties(const Frame &item,
                                             Builder *builder) const {
  builder->Add(n_ref_, item.Id());
  Handle name = item.GetHandle(n_name_);
  if (!name.IsNil()) builder->Add(n_text_, name);
  Handle description = item.GetHandle(n_description_);
  if (!description.IsNil()) builder->Add(n_description_, description);
}

}  // namespace nlp
}  // namespace sling

