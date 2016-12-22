// See LICENSE for license details.

#ifndef Node_H_
#define Node_H_

#ifndef __SYNTHESIS__
#include <cstddef>
#include <cassert>
#endif

class Node {
public:
  unsigned long long next_offset;
  unsigned long long val;

  enum { BitCnt = 128 };

  static size_t getBitCnt() {
    assert(sizeof(Node) == (size_t) BitCnt/8);
    assert( 0 == (size_t) BitCnt%8);
    return BitCnt;
  }
  static size_t numberOfFields() {
    return 2;
  }
  static size_t fieldWidth( size_t index) {
    if ( 0 <= index && index < 1) {
      return 64;
    }
    if ( 1 <= index && index < 2) {
      return 64;
    }
    return 0;
  }
  void putField(size_t index, UInt64 d) {
    if ( 0 <= index && index < 1) {
      next_offset = d;
    }
    if ( 1 <= index && index < 2) {
      val = d;
    }
  }
  UInt64 getField(size_t index) const {
    if ( 0 <= index && index < 1) {
      return next_offset;
    }
    if ( 1 <= index && index < 2) {
      return val;
    }
    return 0;
  }

#if !defined(__AAL_USER__) && !defined(USE_SOFTWARE)
  inline friend void sc_trace(sc_trace_file* tf, const Node& d, const std::string& name) {
  }
#endif

  inline friend std::ostream& operator<<(std::ostream& os, const Node& d) {
    os << "<Node>";
    return os;
  }

  inline bool operator==(const Node& rhs) const {
    bool result = true;
    for( unsigned int i=0; i<numberOfFields(); ++i) {
      result = result && (getField(i) == rhs.getField(i));
    }
    return result;
  }

};

#endif
