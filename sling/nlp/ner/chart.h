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

#ifndef SLING_NLP_NER_CHART_H_
#define SLING_NLP_NER_CHART_H_

#include <unordered_set>
#include <vector>

#include "sling/base/types.h"
#include "sling/frame/store.h"
#include "sling/nlp/document/document.h"
#include "sling/nlp/kb/phrase-table.h"

namespace sling {
namespace nlp {

// Span categorization flags.
enum SpanFlags {
  SPAN_NUMBER         = (1 << 0),
  SPAN_NATURAL_NUMBER = (1 << 1),
  SPAN_YEAR           = (1 << 2),
};

// Stop word list. A span cannot start or end with a stop word.
class StopWords {
 public:
  // Add stop word.
  void Add(Text word);

  // Check if token is a stop word.
  bool Discard(const Token &token) const;

 private:
  // Fingerprints for stop words.
  std::unordered_set<uint64> fingerprints_;
};

// Span chart for sentence in document. This represents all the phrase matches
// up to a maximum length.
class SpanChart {
 public:
  // Chart item.
  struct Item {
    // Phrase matches in phrase table.
    const PhraseTable::Phrase *matches = nullptr;

    // Auxiliary match from annotators.
    Handle aux = Handle::nil();

    // Span cost.
    float cost = 0.0;

    // Optimal split point for item.
    int split = -1;

    // Span flags.
    int flags = 0;
  };

  // Initialize empty span chart for (part of) document.
  SpanChart(Document *document, int begin, int end, int maxlen);

  // Add auxiliary match to chart.
  void Add(int begin, int end, Handle match, int flags = 0);

  // Populate chart with matches from phrase table.
  void Populate(const PhraseTable &phrase_table, const StopWords &stopwords);

  // Compute non-overlapping span covering with minimum cost.
  void Solve();

  // Extract best span covering.
  void Extract();

  // Return item for token span (0 <= begin < size, 0 < end <= size).
  Item &item(int begin, int end) {
    return items_[begin * size_ + end - 1];
  }
  int size() const { return size_; }
  int maxlen() const { return maxlen_; }

  // Get document part for chart.
  Document *document() const { return document_; }
  int begin() const { return begin_; }
  int end() const { return end_; }

 private:
  // Document and token span for chart.
  Document *document_;
  int begin_;
  int end_;

  // Maximum phrase length considered for matching.
  int maxlen_;

  // Chart items indexed by span start and length.
  std::vector<Item> items_;
  int size_;

  // Tracked frame handles.
  Handles tracking_;
};

}  // namespace nlp
}  // namespace sling

#endif  // SLING_NLP_NER_CHART_H_
