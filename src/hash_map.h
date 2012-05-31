// Copyright 2011 Google Inc. All Rights Reserved.
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

#ifndef NINJA_MAP_H_
#define NINJA_MAP_H_

#include "string_piece.h"

// MurmurHash2, by Austin Appleby
static inline
unsigned int MurmurHash2(const void* key, int len) {
  static const unsigned int seed = 0xDECAFBAD;
  const unsigned int m = 0x5bd1e995;
  const int r = 24;
  unsigned int h = seed ^ len;
  const unsigned char * data = (const unsigned char *)key;
  while(len >= 4) {
    unsigned int k = *(unsigned int *)data;
    k *= m;
    k ^= k >> r;
    k *= m;
    h *= m;
    h ^= k;
    data += 4;
    len -= 4;
  }
  switch(len) {
  case 3: h ^= data[2] << 16;
  case 2: h ^= data[1] << 8;
  case 1: h ^= data[0];
    h *= m;
  };
  h ^= h >> 13;
  h *= m;
  h ^= h >> 15;
  return h;
}

#if defined(_MSC_VER)
#define BIG_CONSTANT(x) (x)
#else   // defined(_MSC_VER)
#define BIG_CONSTANT(x) (x##LLU)
#endif // !defined(_MSC_VER)
static inline
uint64_t MurmurHash64A(const void* key, int len) {
  static const uint64_t seed = 0xDECAFBADDECAFBADllu;
  const uint64_t m = BIG_CONSTANT(0xc6a4a7935bd1e995);
  const int r = 47;
  uint64_t h = seed ^ (len * m);
  const uint64_t * data = (const uint64_t *)key;
  const uint64_t * end = data + (len/8);
  while(data != end) {
    uint64_t k = *data++;
    k *= m; 
    k ^= k >> r; 
    k *= m; 
    h ^= k;
    h *= m; 
  }
  const unsigned char* data2 = (const unsigned char*)data;
  switch(len & 7)
  {
  case 7: h ^= uint64_t(data2[6]) << 48;
  case 6: h ^= uint64_t(data2[5]) << 40;
  case 5: h ^= uint64_t(data2[4]) << 32;
  case 4: h ^= uint64_t(data2[3]) << 24;
  case 3: h ^= uint64_t(data2[2]) << 16;
  case 2: h ^= uint64_t(data2[1]) << 8;
  case 1: h ^= uint64_t(data2[0]);
          h *= m;
  };
  h ^= h >> r;
  h *= m;
  h ^= h >> r;
  return h;
} 
#undef BIG_CONSTANT

#ifdef _MSC_VER
#include <hash_map>

using stdext::hash_map;
using stdext::hash_compare;

struct StringPieceCmp : public hash_compare<StringPiece> {
  size_t operator()(const StringPiece& key) const {
    return MurmurHash2(key.str_, key.len_);
  }
  bool operator()(const StringPiece& a, const StringPiece& b) const {
    int cmp = strncmp(a.str_, b.str_, min(a.len_, b.len_));
    if (cmp < 0) {
      return true;
    } else if (cmp > 0) {
      return false;
    } else {
      return a.len_ < b.len_;
    }
  }
};

#else

#include <ext/hash_map>

using __gnu_cxx::hash_map;

namespace __gnu_cxx {
template<>
struct hash<std::string> {
  size_t operator()(const std::string& s) const {
    return hash<const char*>()(s.c_str());
  }
};

template<>
struct hash<StringPiece> {
  size_t operator()(StringPiece key) const {
    return MurmurHash2(key.str_, key.len_);
  }
};

}
#endif

/// A template for hash_maps keyed by a StringPiece whose string is
/// owned externally (typically by the values).  Use like:
/// ExternalStringHash<Foo*>::Type foos; to make foos into a hash
/// mapping StringPiece => Foo*.
template<typename V>
struct ExternalStringHashMap {
#ifdef _MSC_VER
  typedef hash_map<StringPiece, V, StringPieceCmp> Type;
#else
  typedef hash_map<StringPiece, V> Type;
#endif
};

#endif // NINJA_MAP_H_
