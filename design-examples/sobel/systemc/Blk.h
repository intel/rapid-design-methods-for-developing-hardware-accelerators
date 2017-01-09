// See LICENSE for license details.

#ifndef Blk_H_
#define Blk_H_

#ifndef __SYNTHESIS__
#include <cstddef>
#include <cassert>
#endif

template<typename T, int N>
class Blk {
public:
  T data[N];

  enum { ArrayLength = N };
  typedef T ElementType;

  enum { BitCnt = 8*sizeof(T)*N };

  static size_t getBitCnt() {
    assert(sizeof(Blk) == (size_t) BitCnt/8);
    assert( 0 == (size_t) BitCnt%8);
    return BitCnt;
  }
  static size_t numberOfFields() {
    return N;
  }
  static size_t fieldWidth( size_t index) {
    return 8*sizeof(T);
  }
  void putField(size_t index, UInt64 d) {
    data[index] = d;
  }
  UInt64 getField(size_t index) const {
    return data[index];
  }

#if !defined(__AAL_USER__) && !defined(USE_SOFTWARE)
  inline friend void sc_trace(sc_trace_file* tf, const Blk<T,N>& d, const std::string& name) {
  }
#endif

  inline friend std::ostream& operator<<(std::ostream& os, const Blk<T,N>& d) {
    os << "<Blk>";
    return os;
  }

  inline bool operator==(const Blk& rhs) const {
    bool result = true;
    for( unsigned int i=0; i<numberOfFields(); ++i) {
      result = result && (getField(i) == rhs.getField(i));
    }
    return result;
  }

  Blk() {} // the default constructor should not initialize the array

  Blk( const T& elem) {
  UNROLL_INITIALIZE_BLK:
    for( unsigned int i=0; i<N; ++i) {
      data[i] = elem;
    }
  }

  inline friend Blk<T,N> operator+( const Blk<T,N>& a, const Blk<T,N>& b) {
    Blk result;
  UNROLL_SUM:
    for( unsigned int i=0; i<N; ++i) {
      result.data[i] = a.data[i] + b.data[i];
    }
    return result;
  }

  inline friend Blk<T,N> operator*( const Blk<T,N>& a, const T& k) {
    Blk result;
  UNROLL_PRODUCT:
    for( unsigned int i=0; i<N; ++i) {
      result.data[i] = a.data[i] * k;
    }
    return result;
  }

};

#endif
