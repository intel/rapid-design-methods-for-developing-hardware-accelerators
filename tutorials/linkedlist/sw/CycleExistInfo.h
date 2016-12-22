// See LICENSE for license details.

#ifndef CycleExistInfo_H_
#define CycleExistInfo_H_

#ifndef __SYNTHESIS__
#include <cstddef>
#include <cassert>
#endif

class CycleExistInfo {
public:
  unsigned int info : 32;

  CycleExistInfo() {}
  CycleExistInfo(bool r) :
    info(r) {}
    
  enum { BitCnt = 32 };

  static size_t getBitCnt() {
    assert(sizeof(CycleExistInfo) == (size_t) BitCnt/8);
    assert( 0 == (size_t) BitCnt%8);
    return BitCnt;
  }
  static size_t numberOfFields() {
    return 1;
  }
  static size_t fieldWidth( size_t index) {
    if ( 0 <= index && index < 1) {
      return 32;
    }
    return 0;
  }
  void putField(size_t index, UInt64 d) {
    if ( 0 <= index && index < 1) {
      info = d;
    }
  }
  UInt64 getField(size_t index) const {
    if ( 0 <= index && index < 1) {
      return info;
    }
    return 0;
  }

#if !defined(__AAL_USER__) && !defined(USE_SOFTWARE)
  inline friend void sc_trace(sc_trace_file* tf, const CycleExistInfo& d, const std::string& name) {
  }
#endif

  inline friend std::ostream& operator<<(std::ostream& os, const CycleExistInfo& d) {
    os << "<CycleExistInfo>";
    return os;
  }

  inline bool operator==(const CycleExistInfo& rhs) const {
    bool result = true;
    for( unsigned int i=0; i<numberOfFields(); ++i) {
      result = result && (getField(i) == rhs.getField(i));
    }
    return result;
  }

};

#endif
