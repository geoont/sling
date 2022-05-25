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

#ifndef SLING_UTIL_ARENA_H_
#define SLING_UTIL_ARENA_H_

#include <string.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "sling/string/text.h"

namespace sling {

// Arena allocator for allocating memory from larger memory regions. Arena
// allocation is fast, has low overhead, and all the objects in an arena can be
// efficiently deallocated all at once.
template<typename T = char> class Arena {
 public:
  // Initialize arena.
  Arena(size_t chunk = 1 << 20) : chunk_(chunk) {}

  // Deallocate arena.
  ~Arena() { for (T *r : regions_) free(r); }

  // Allocate memory from arena.
  T *alloc(size_t size = 1) {
    size_t bytes = size * sizeof(T);
    if (free_ < bytes) expand();
    T *ptr = heap_;
    heap_ += size;
    free_ -= bytes;
    return ptr;
  }

  // Allocate memory initialized from another memory object.
  T *dup(const T *data, size_t size = 1) {
    T *ptr = alloc(size);
    memcpy(ptr, data, size * sizeof(T));
    return ptr;
  }

  // Deallocate all objects from arena.
  void clear() {
    for (T *r : regions_) free(r);
    regions_.clear();
    heap_ = nullptr;
    free_ = 0;
  }

  // Number of bytes allocated by arena.
  size_t size() {
    return regions_.size() * chunk_ * sizeof(T);
  }

 private:
  // Allocate a new region.
  void expand() {
    T *memory = static_cast<T *>(malloc(chunk_ * sizeof(T)));
    heap_ = memory;
    free_ = chunk_ * sizeof(T);
    regions_.push_back(memory);
  }

  // Pointer to the unused part of the current region.
  T *heap_ = nullptr;

  // Bytes remaining in the unallocated part of of the current region.
  size_t free_ = 0;

  // Size of each region.
  size_t chunk_;

  // List of allocated regions.
  std::vector<T *> regions_;
};

// Arena for allocating nul-terminated strings.
class StringArena : public Arena<char> {
 public:
  // Duplicate nul-terminated string.
  char *dup(const char *str) {
    return Arena::dup(str, strlen(str) + 1);
  }

  // Create nul-terminated string from memory block.
  char *dup(const char *str, size_t size) {
    char *ptr = alloc(size + 1);
    memcpy(ptr, str, size);
    ptr[size] = 0;
    return ptr;
  }

  // Allocate nul-terminated string from string object.
  char *dup(const std::string &str) {
    return dup(str.data(), str.size());
  }

  // Allocate nul-terminated string from slice object.
  char *dup(const Slice &slice) {
    return dup(slice.data(), slice.size());
  }

  // Allocate space for text object.
  Text dup(Text str) {
    return Text(dup(str.data(), str.size()), str.size());
  }
};

}  // namespace sling

#endif  // SLING_UTIL_ARENA_H_

