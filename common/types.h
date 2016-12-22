// See LICENSE for license details.

#ifndef __ACC_TYPES__
#define __ACC_TYPES__

#ifndef __SYNTHESIS__
#include <iomanip>
#endif

#include "systemc.h"
//#define DEBUG_OUTPUT

#ifdef DEBUG_OUTPUT
#define DBG_OUT std::cout
#else
#define DBG_OUT if (0) std::cout
#endif

#define BIT(n)                  ( (n>=64)?0:(UInt64)1<<(n) )
#define BIT_SET(y, mask)        ( y |=  (mask) )
#define BIT_CLEAR(y, mask)      ( y &= ~(mask) )
#define BIT_FLIP(y, mask)       ( y ^=  (mask) )

//! Create a bitmask of length \a len.
#define BIT_MASK(len)           ( BIT((UInt64)len)-1 )
//! Create a bitfield mask of length \a starting at bit \a start.
#define BF_MASK(start, len)     ( BIT_MASK((UInt64)len)<<((UInt64)start) )
//! Prepare a bitmask for insertion or combining.
#define BF_PREP(x, start, len)  ( (((UInt64)x)&BIT_MASK((UInt64)len)) << ((UInt64)start) )
#define LEN(start, end) ((end) - (start) + 1)
//! Extract a bitfield of length \a len starting at bit \a start from \a y.
#define BF_GET(y, end, start)   ( ((y)>>((UInt64)start)) & BIT_MASK(LEN((UInt64)start, (UInt64)end)) )
//! Insert a new bitfield value \a x into \a y.
#define BF_SET(y, x,end, start)  ( y= ((y) &~ BF_MASK((UInt64)start, LEN((UInt64)start, (UInt64)end)) ) | BF_PREP(x, (UInt64)start, LEN((UInt64)start, (UInt64)end)) )

#define LAST(k,n) ((k) & (((n>=64)?0:(UInt64)1<<(n))-1))
#define MID(k,m,n) LAST((k)>>(m),((n)-(m)))
#define BIT_RANGE(data, a, b) MID(data,(UInt64)b,(UInt64)(a+1))

struct AccNoType {};

#ifdef USE_HLS


typedef sc_uint<1> UInt1 ;
typedef sc_uint<2> UInt2 ;
typedef sc_uint<3> UInt3 ;
typedef sc_uint<4> UInt4 ;
typedef sc_uint<5> UInt5 ;
typedef sc_uint<6> UInt6 ;
typedef sc_uint<7> UInt7 ;
typedef sc_uint<8> UInt8 ;
typedef sc_uint<9> UInt9 ;
typedef sc_uint<10> UInt10 ;
typedef sc_uint<12> UInt12 ;
typedef sc_uint<16> UInt16 ;
typedef sc_uint<32> UInt32 ;
typedef sc_uint<64> UInt64 ;

typedef sc_int<7> Int7 ;
typedef sc_int<8> Int8 ;
typedef sc_int<16> Int16 ;

template <size_t MAX_VALUE_PLUS_1>
struct SizeT {
  typedef UInt64 Type;
};

#include "hls_types.h"
// in CTOS the SizeT::Type based on recursive meta computation of log2 is not allowed :(
template <> struct SizeT<256> {typedef UInt8 Type;};
template <> struct SizeT<128> {typedef UInt7 Type;};
template <> struct SizeT<64>  {typedef UInt6 Type;};
template <> struct SizeT<32>  {typedef UInt5 Type;};
template <> struct SizeT<16>  {typedef UInt4 Type;};
template <> struct SizeT<9>   {typedef UInt4 Type;};
template <> struct SizeT<8>   {typedef UInt3 Type;};
template <> struct SizeT<7>   {typedef UInt3 Type;};
template <> struct SizeT<6>   {typedef UInt3 Type;};
template <> struct SizeT<5>   {typedef UInt3 Type;};
template <> struct SizeT<4>   {typedef UInt2 Type;};
template <> struct SizeT<3>   {typedef UInt2 Type;};
template <> struct SizeT<2>   {typedef UInt1 Type;};
// this really means you have a constant 0 but i'll still put it in the UInt1 for generality
template <> struct SizeT<1>   {typedef UInt1 Type;};

#include "hls_utils.h"

#else // #ifdef USE_HLS

#include <stdint.h>
#include "systemc.h"
#ifndef STRATUS
#include "sysc/datatypes/fx/sc_fixed.h"
#include "sysc/datatypes/fx/sc_ufixed.h"
#endif

template <size_t MAX_VALUE_PLUS_1>
struct SizeT {
  typedef uint64_t Type;
};

typedef uint8_t UInt2 ;
typedef uint8_t UInt3 ;
typedef uint8_t UInt4 ;
typedef uint8_t UInt5 ;
typedef uint8_t UInt6 ;
typedef uint8_t UInt7 ;
typedef uint8_t UInt8 ;
typedef uint16_t UInt9 ;
typedef uint16_t UInt10 ;

typedef uint16_t UInt12 ;
typedef uint16_t UInt16 ;
typedef uint32_t UInt32 ;
typedef uint64_t UInt64 ;

typedef int8_t Int7 ;
typedef int8_t Int8 ;
typedef int16_t Int16 ;

#define REAL_VAL_TYPE(___I, ___F) double
#define UREAL_VAL_TYPE(___I, ___F) double


#endif // #ifdef USE_HLS



typedef UInt32 SizeInCLType;
typedef UInt32 MemRequestSizeType;
//typedef UInt16 DataRequestCountType;
// this may need to change depending on what the address type is (64 vs 58 bits)
typedef UInt64 AddressType;

typedef UInt9 SplTag;
#define ADDRESS_TYPE_BIT_WIDTH 64

typedef AddressType SplAddressType;

struct  CacheLineType {
  unsigned long long int words[8];
  //UInt64 words[8];


  enum {
    BitCnt = 512
  };
  static size_t getBitCnt() {
    return BitCnt;
  }

  // should be able to hold the max value as well, so one bit larger
  typedef UInt10 IndexType;
  typedef UInt7 ByteIndexType;
  enum {
    BYTE_INDEX_MAX_VALUE = 63
  };
  //static const ByteIndexType BYTE_INDEX_MAX_VALUE = 63;
//  inline friend void sc_trace(sc_trace_file* tf, const CacheLineType& d,
//      const std::string& name) {
//  }
//
  CacheLineType () {
    words[0] = words[1] = words[2] = words[3] = words[4] = words[5] = words[6] = words[7] = 0xDEADBEEFDEADBEEF;
  }

  CacheLineType (const CacheLineType &rhs) {
    copy(rhs);
  }


  CacheLineType (UInt64 w0, UInt64 w1, UInt64 w2, UInt64 w3, UInt64 w4, UInt64 w5, UInt64 w6, UInt64 w7) {
    words[0] = w0;
    words[1] = w1;
    words[2] = w2;
    words[3] = w3;
    words[4] = w4;
    words[5] = w5;
    words[6] = w6;
    words[7] = w7;
  }

  static CacheLineType::ByteIndexType getCacheLineOffset(AddressType addr) {
    return addr & CacheLineType::BYTE_INDEX_MAX_VALUE;
  }
  static AddressType getCacheAlignedAddress(AddressType addr) {
    return addr & ~((AddressType)CacheLineType::BYTE_INDEX_MAX_VALUE);
  }

  inline friend std::ostream& operator<<(std::ostream& os,
      const CacheLineType& d) {
#ifndef __SYNTHESIS__
    for (unsigned int i=0; i<8; ++i) {
      os << "0x" << std::setfill('0') << std::setw(16) << std::hex << d.words[i];
      if ( i < 7) { os << " "; }
    }
    os << std::dec;
#endif
    return os;
  }

  inline bool operator==(const CacheLineType& rhs) const {
    bool result = true;
    UNROLL_EQ_WORDS0: for (int words_0 = 0; words_0 < 8; ++words_0) {
      result = result && (words[words_0] == rhs.words[words_0]);
    }
    return result;
  }

  CacheLineType& operator=(const CacheLineType& rhs) {
    copy(rhs);
    return *this;
  }

  static CacheLineType generate_random() {
    CacheLineType cl;
    for (int cli = 0; cli < 8; ++cli) {
        cl.words[cli] = rand();
    }
    return cl;
  }

  void clear() {
    CL_CLEAR_UNROLL_FOR: for (unsigned i = 0; i < getBitCnt()/64; ++i) {
      words[i] = 0;
    }
  }

  static SizeInCLType computeCLSize(UInt64 total_size) {
    if (total_size <= CacheLineType::getBitCnt()) return 1;
    else {
      return (SizeInCLType)((total_size / CacheLineType::getBitCnt())) + ((total_size % CacheLineType::getBitCnt())? 1 : 0);
    }
  }

  // range from (end:start), e.g. [63:0] would be words[0]
  UInt64 getDataInRange(CacheLineType::IndexType start, CacheLineType::IndexType width) const {
    assert(width <= 64);
    CacheLineType::IndexType index = BIT_RANGE(start,9,6);
    CacheLineType::IndexType word_start = BIT_RANGE(start,5,0);
    assert(word_start+width <= 64);
    return BIT_RANGE(words[index], word_start+width-1, word_start);
  }

  void putDataInRange(CacheLineType::IndexType start, CacheLineType::IndexType width, UInt64 data) {
    assert(width <= 64);
    CacheLineType::IndexType index = BIT_RANGE(start,9,6);
    CacheLineType::IndexType word_start = BIT_RANGE(start,5,0);
    assert(word_start+width <= 64);
    BF_SET(words[index], data, word_start+width-1, word_start);
  }
  static size_t numberOfFields() {
    return 8;
  }

  static size_t fieldWidth(size_t index) {
    return BitCnt/8;
  }

  void putField(size_t index, UInt64 d) {
    words[index] = d;
  }

  UInt64 getField(size_t index) const {
    return words[index];
  }

#ifdef STRATUS
  inline friend void cynw_interpret( const CacheLineType& inp,
                                     sc_bv<512>& out) {
    size_t fw = CacheLineType::fieldWidth(0);
    for(unsigned i=0; i<CacheLineType::numberOfFields(); ++i) {
      out.range( fw*i+fw-1, fw*i) = inp.getField( i);
    }
  }

  inline friend void cynw_interpret( const sc_bv<512>& inp,
                                     CacheLineType&    out) {
    size_t fw = CacheLineType::fieldWidth(0);
    for(unsigned i=0; i<CacheLineType::numberOfFields(); ++i) {
      out.putField( i, inp.range( fw*i+fw-1, fw*i).to_uint64());
    }
  }                                     
#endif

private:
  void copy(const CacheLineType& rhs) {
    UNROLL_COPY_WORDS0: for (int words_0 = 0; words_0 < 8; ++words_0) {
      words[words_0] = rhs.words[words_0];
    }
  }
};

struct AddressUtils {
  static SplAddressType getNextCLAddress(SplAddressType addr) {
    return addr + CacheLineType::getBitCnt()/8;
  }

};

typedef UInt7 IOUnitIdType;
// mem read types
struct AccMemReadReqType {
  AddressType addr;
  SizeInCLType size;

  AccMemReadReqType () {}
  AccMemReadReqType (AddressType addr, SizeInCLType size) :
    addr(addr), size(size) {}

  inline friend void sc_trace(sc_trace_file* tf, const AccMemReadReqType& d,
      const std::string& name) {
  }

  inline friend std::ostream& operator<<(std::ostream& os,
      const AccMemReadReqType& d) {
    os << "addr: " << d.addr << std::endl;
    os << "size: " << d.size << std::endl;
    return os;
  }

  inline bool operator==(const AccMemReadReqType& rhs) {
    bool result = true;
    result = result && (addr == rhs.addr);
    result = result && (size == rhs.size);
    return result;
  }

};

struct AccMemReadRespType {
  CacheLineType data;

  AccMemReadRespType(){}
  AccMemReadRespType(const AccMemReadRespType& rhs){
    copy(rhs);
  }
  AccMemReadRespType(const CacheLineType &cl) : data(cl) {}

  inline friend void sc_trace(sc_trace_file* tf, const AccMemReadRespType& d,
      const std::string& name) {
  }

  inline friend std::ostream& operator<<(std::ostream& os,
      const AccMemReadRespType& d) {
    os << "data: " << d.data << std::endl;
    return os;
  }

  inline bool operator==(const AccMemReadRespType& rhs) const {
    bool result = true;
    result = result && (data == rhs.data);
    return result;
  }

  AccMemReadRespType& operator=(const AccMemReadRespType& rhs) {
    copy(rhs);
    return *this;
  }
private:
  void copy(const AccMemReadRespType& rhs) {
    data = rhs.data;
  }
};

template<typename T>
struct MemTypedWriteReqType {
  AddressType addr;
  MemRequestSizeType size;
  MemTypedWriteReqType() {}
  MemTypedWriteReqType(AddressType addr, MemRequestSizeType size) :
    addr(addr), size(size)
  {}

  inline friend void sc_trace(sc_trace_file* tf, const MemTypedWriteReqType& d,
      const std::string& name) {
  }

  inline friend std::ostream& operator<<(std::ostream& os,
      const MemTypedWriteReqType& d) {
    os << "addr: " << d.addr << std::endl;
    os << "size: " << d.size << std::endl;
    return os;
  }

  inline bool operator==(const MemTypedWriteReqType& rhs) {
    bool result = true;
    result = result && (addr == rhs.addr);
    result = result && (size == rhs.size);
    return result;
  }
};

template<typename T>
struct MemTypedWriteDataType {
  T data;
  MemTypedWriteDataType() {}
  MemTypedWriteDataType(const MemTypedWriteDataType& rhs) {
    copy(rhs);
  }
  MemTypedWriteDataType(const T &d) :
    data(d)
  {}

  inline friend void sc_trace(sc_trace_file* tf, const MemTypedWriteDataType& d,
      const std::string& name) {
  }

  inline friend std::ostream& operator<<(std::ostream& os,
      const MemTypedWriteDataType& d) {
    os << "data: " << d.data << std::endl;
    return os;
  }

  inline bool operator==(const MemTypedWriteDataType& rhs) {
    bool result = true;
    result = result && (data == rhs.data);
    return result;
  }

  MemTypedWriteDataType& operator=(const MemTypedWriteDataType& rhs) {
    copy(rhs);
    return *this;
  }
private:
  void copy(const MemTypedWriteDataType &rhs) {
    data = rhs.data;
  }
};

template<typename T>
struct MemTypedReadReqType {
  AddressType addr;
  MemRequestSizeType size;

  MemTypedReadReqType() {}

  MemTypedReadReqType(AddressType addr, MemRequestSizeType size) :
    addr(addr), size(size) {}

  inline friend void sc_trace(sc_trace_file* tf, const MemTypedReadReqType& d,
      const std::string& name) {
  }

  inline friend std::ostream& operator<<(std::ostream& os,
      const MemTypedReadReqType& d) {
    os << "addr: " << d.addr << std::endl;
    os << "size: " << d.size << std::endl;
    return os;
  }

  inline bool operator==(const MemTypedReadReqType& rhs) const {
    bool result = true;
    result = result && (addr == rhs.addr);
    result = result && (size == rhs.size);
    return result;
  }

};

template<typename T>
struct MemTypedReadRespType {
  T data;

  MemTypedReadRespType() {}
  MemTypedReadRespType(const MemTypedReadRespType& rhs) {
    copy(rhs);
  }

  MemTypedReadRespType(const T& d) :
    data(d) {}

  inline friend void sc_trace(sc_trace_file* tf, const MemTypedReadRespType& d,
      const std::string& name) {
  }

  inline friend std::ostream& operator<<(std::ostream& os,
      const MemTypedReadRespType& d) {
    os << "data: " << d.data << std::endl;
    return os;
  }

  inline bool operator==(const MemTypedReadRespType& rhs) const {
    bool result = true;
    result = result && (data == rhs.data);
    return result;
  }

  MemTypedReadRespType& operator=(const MemTypedReadRespType& rhs) {
    copy(rhs);
    return *this;
  }
private:
  void copy(const MemTypedReadRespType &rhs) {
    data = rhs.data;
  }

};

template<typename T, typename UTAG>
struct MemSingleWriteReqType {
  AddressType addr;
  T data;
  UTAG utag;

  MemSingleWriteReqType() {}

  MemSingleWriteReqType(AddressType addr, UTAG utag, const T &data) :
    addr(addr), data(data), utag(utag) {}

  MemSingleWriteReqType(AddressType addr, const T &data) :
    addr(addr), data(data) {}

  inline friend void sc_trace(sc_trace_file* tf, const MemSingleWriteReqType& d,
      const std::string& name) {
  }

  inline friend std::ostream& operator<<(std::ostream& os,
      const MemSingleWriteReqType& d) {
    os << "addr: " << d.addr << std::endl;
    os << "utag: " << d.utag << std::endl;
    os << "data: " << d.data << std::endl;
    return os;
  }

  inline bool operator==(const MemSingleWriteReqType& rhs) const {
    bool result = true;
    result = result && (addr == rhs.addr);
    result = result && (utag == rhs.utag);
    result = result && (data == rhs.data);
    return result;
  }

};


template<typename T, typename UTAG>
struct MemSingleReadReqType {
  AddressType addr;
  UTAG utag;

  MemSingleReadReqType() {}

  MemSingleReadReqType(AddressType addr, UTAG utag) :
    addr(addr), utag(utag) {}

  inline friend void sc_trace(sc_trace_file* tf, const MemSingleReadReqType& d,
      const std::string& name) {
  }

  inline friend std::ostream& operator<<(std::ostream& os,
      const MemSingleReadReqType& d) {
    os << "addr: " << d.addr << std::endl;
    os << "utag: " << d.utag << std::endl;
    return os;
  }

  inline bool operator==(const MemSingleReadReqType& rhs) const {
    bool result = true;
    result = result && (addr == rhs.addr);
    result = result && (utag == rhs.utag);
    return result;
  }

};

template<typename T, typename UTAG>
struct MemSingleReadWriteReqType {
  AddressType addr;
  T data;
  UTAG utag;
  bool is_write;

  MemSingleReadWriteReqType(): addr(0), data(), utag(0), is_write(false) {}

  MemSingleReadWriteReqType(MemSingleWriteReqType<T, UTAG> wr_req) :
    addr(wr_req.addr), data(wr_req.data), utag(wr_req.utag), is_write(true) {}

  MemSingleReadWriteReqType(MemSingleReadReqType<T, UTAG> rd_req) :
    addr(rd_req.addr), data(), utag(rd_req.utag), is_write(false) {}

  inline friend void sc_trace(sc_trace_file* tf, const MemSingleReadWriteReqType& d,
      const std::string& name) {
  }

  inline friend std::ostream& operator<<(std::ostream& os,
      const MemSingleReadWriteReqType& d) {
    os << "addr: " << d.addr << " utag: " << d.utag <<  " data: " << d.data << " is_write " << d.is_write << std::endl;
    return os;
  }

  inline bool operator==(const MemSingleReadWriteReqType& rhs) const {
    bool result = true;
    result = result && (addr == rhs.addr);
    result = result && (utag == rhs.utag);
    result = result && (data == rhs.data);
    result = result && (is_write == rhs.is_write);
    return result;
  }
};



template<typename T, typename UTAG>
struct MemSingleReadRespType {
  T data;
  UTAG utag;

  MemSingleReadRespType() {}
  MemSingleReadRespType(const MemSingleReadRespType& rhs) {
    copy(rhs);
  }

  MemSingleReadRespType(const T& d, const UTAG& utag) :
    data(d), utag(utag) {}

  inline friend void sc_trace(sc_trace_file* tf, const MemSingleReadRespType& d,
      const std::string& name) {
  }

  inline friend std::ostream& operator<<(std::ostream& os,
      const MemSingleReadRespType& d) {
    os << "data: " << d.data << std::endl;
    os << "tag: " << d.utag << std::endl;
    return os;
  }

  inline bool operator==(const MemSingleReadRespType& rhs) const {
    bool result = true;
    result = result && (data == rhs.data);
    result = result && (utag == rhs.utag);
    return result;
  }

  MemSingleReadRespType& operator=(const MemSingleReadRespType& rhs) {
    copy(rhs);
    return *this;
  }
private:
  void copy(const MemSingleReadRespType &rhs) {
    data = rhs.data;
    utag = rhs.utag;
  }

};

template<typename UTAG>
struct MemSingleWriteRespType {
  UTAG utag;

  MemSingleWriteRespType() {}
  MemSingleWriteRespType(const MemSingleWriteRespType& rhs) {
    copy(rhs);
  }

  MemSingleWriteRespType(const UTAG& utag) :
    utag(utag) {}

  inline friend void sc_trace(sc_trace_file* tf, const MemSingleWriteRespType& d,
      const std::string& name) {
  }

  inline friend std::ostream& operator<<(std::ostream& os,
      const MemSingleWriteRespType& d) {
    os << "tag: " << d.utag << std::endl;
    return os;
  }

  inline bool operator==(const MemSingleWriteRespType& rhs) const {
    bool result = true;
    result = result && (utag == rhs.utag);
    return result;
  }

  MemSingleWriteRespType& operator=(const MemSingleWriteRespType& rhs) {
    copy(rhs);
    return *this;
  }
private:
  void copy(const MemSingleWriteRespType &rhs) {
    utag = rhs.utag;
  }

};


struct SplMemReadReqType {
  SplAddressType addr;
  IOUnitIdType io_unit_id;
  SplTag tag;

  SplMemReadReqType() {}
  SplMemReadReqType(SplAddressType addr) : addr(addr)
  {
  }

  SplMemReadReqType(SplAddressType addr, IOUnitIdType io_unit_id, SplTag tag) :
    addr(addr), io_unit_id(io_unit_id), tag(tag)
  {
  }

  inline friend void sc_trace(sc_trace_file* tf, const SplMemReadReqType& d,
      const std::string& name) {
  }

  inline friend std::ostream& operator<<(std::ostream& os,
      const SplMemReadReqType& d) {
    os << "addr: " << d.addr << std::endl;
    os << "io_unit_id: " << d.io_unit_id << std::endl;
    return os;
  }

  inline bool operator==(const SplMemReadReqType& rhs) {
    bool result = true;
    result = result && (addr == rhs.addr);
    result = result && (io_unit_id == rhs.io_unit_id);
    result = result && (tag == rhs.tag);
    return result;
  }

};
struct SplMemReadRespType {
  CacheLineType data;
  IOUnitIdType io_unit_id;
  SplTag tag;

  SplMemReadRespType() {}

  SplMemReadRespType(const SplMemReadRespType& rhs) {
      copy(rhs);
  }

  SplMemReadRespType(CacheLineType data, IOUnitIdType io_unit_id, SplTag tag) :
    data(data), io_unit_id(io_unit_id), tag(tag) {}

  SplMemReadRespType(CacheLineType data, IOUnitIdType io_unit_id) :
    data(data), io_unit_id(io_unit_id) {}

  inline friend void sc_trace(sc_trace_file* tf, const SplMemReadRespType& d,
      const std::string& name) {
  }

  inline friend std::ostream& operator<<(std::ostream& os,
      const SplMemReadRespType& d) {
    os << "data: " << d.data << std::endl;
    os << "io_unit_id: " << d.io_unit_id << std::endl;
    return os;
  }

  inline bool operator==(const SplMemReadRespType& rhs) {
    bool result = true;
    result = result && (data == rhs.data);
    result = result && (io_unit_id == rhs.io_unit_id);
    result = result && (tag == rhs.tag);
    return result;
  }

  SplMemReadRespType& operator=(const SplMemReadRespType& rhs) {
    copy(rhs);
    return *this;
  }
private:
  void copy(const SplMemReadRespType &rhs) {
    data = rhs.data;
    io_unit_id = rhs.io_unit_id;
    tag = rhs.tag;
  }

};

// mem write types

struct AccMemWriteReqType {
  AddressType addr;
  SizeInCLType size;

  AccMemWriteReqType() {}
  AccMemWriteReqType(AddressType addr, SizeInCLType size) :
    addr(addr), size(size) {}

  inline friend void sc_trace(sc_trace_file* tf, const AccMemWriteReqType& d,
      const std::string& name) {
  }

  inline friend std::ostream& operator<<(std::ostream& os,
      const AccMemWriteReqType& d) {
    os << "addr: " << d.addr << std::endl;
    os << "size: " << d.size << std::endl;
    return os;
  }

  inline bool operator==(const AccMemWriteReqType& rhs) {
    bool result = true;
    result = result && (addr == rhs.addr);
    result = result && (size == rhs.size);
    return result;
  }


};
struct AccMemWriteDataType {
  CacheLineType data;
  CacheLineType::ByteIndexType offset;
  CacheLineType::ByteIndexType width;
  AccMemWriteDataType() :
    offset(0), width(CacheLineType::BYTE_INDEX_MAX_VALUE) {}
  AccMemWriteDataType(const AccMemWriteDataType& rhs) {
    copy(rhs);
  }
  AccMemWriteDataType(const CacheLineType &data) : data(data), offset(0), width(CacheLineType::BYTE_INDEX_MAX_VALUE) {}
  AccMemWriteDataType(const CacheLineType &data, CacheLineType::ByteIndexType offset, CacheLineType::ByteIndexType width) :
    data(data), offset(offset), width(width) {}

  inline friend void sc_trace(sc_trace_file* tf, const AccMemWriteDataType& d,
      const std::string& name) {
  }

  inline friend std::ostream& operator<<(std::ostream& os,
      const AccMemWriteDataType& d) {
    os << "data: " << d.data << std::endl;
    return os;
  }

  inline bool operator==(const AccMemWriteDataType& rhs) {
    bool result = true;
    result = result && (data == rhs.data);
    result = result && (offset == rhs.offset);
    result = result && (width == rhs.width);
    return result;
  }

  AccMemWriteDataType &operator=(const AccMemWriteDataType& rhs) {
    copy(rhs);
    return *this;
  }
private:
  void copy(const AccMemWriteDataType& rhs) {
    data = rhs.data;
    offset = rhs.offset;
    width = rhs.width;
  }

};

struct AccMemWriteRespType {
  IOUnitIdType io_unit_id;
};

struct SplMemWriteReqType {
  SplAddressType addr;
  CacheLineType data;
  IOUnitIdType io_unit_id;
  CacheLineType::ByteIndexType offset;
  CacheLineType::ByteIndexType width;
  SplTag tag;

  SplMemWriteReqType(SplAddressType addr, SplTag tag, CacheLineType data) :
    addr(addr), data(data), offset(0), width(CacheLineType::BYTE_INDEX_MAX_VALUE), tag(tag) {}

  SplMemWriteReqType(SplAddressType addr, CacheLineType data) :
    addr(addr), data(data), offset(0), width(CacheLineType::BYTE_INDEX_MAX_VALUE) {}


  SplMemWriteReqType(SplAddressType addr) :
    addr(addr), offset(0), width(CacheLineType::BYTE_INDEX_MAX_VALUE) {}

  SplMemWriteReqType(const SplMemWriteReqType &rhs) {
    copy(rhs);
  }

  SplMemWriteReqType() :
    offset(0), width(CacheLineType::BYTE_INDEX_MAX_VALUE) {}

  inline friend void sc_trace(sc_trace_file* tf, const SplMemWriteReqType& d,
      const std::string& name) {
  }

  inline friend std::ostream& operator<<(std::ostream& os,
      const SplMemWriteReqType& d) {
    os << "addr: " << d.addr << std::endl;
    os << "data: " << d.data << std::endl;
    os << "io_unit_id: " << d.io_unit_id << std::endl;
    return os;
  }

  inline bool operator==(const SplMemWriteReqType& rhs) {
    bool result = true;
    result = result && (addr == rhs.addr);
    result = result && (data == rhs.data);
    result = result && (io_unit_id == rhs.io_unit_id);
    result = result && (offset == rhs.offset);
    result = result && (width == rhs.width);
    result = result && (tag == rhs.tag);
    return result;
  }

  SplMemWriteReqType& operator=(const SplMemWriteReqType& rhs) {
    copy(rhs);
    return *this;
  }
private:
  void copy(const SplMemWriteReqType &rhs) {
    addr = rhs.addr;
    data = rhs.data;
    io_unit_id = rhs.io_unit_id;
    offset = rhs.offset;
    width = rhs.width;
    tag = rhs.tag;
  }
};
struct SplMemWriteRespType {
  bool ack;
  IOUnitIdType io_unit_id;
  SplTag tag;

  SplMemWriteRespType() {}
  SplMemWriteRespType(IOUnitIdType io_unit_id, SplTag tag) :
    io_unit_id(io_unit_id), tag(tag) {}

  inline friend void sc_trace(sc_trace_file* tf, const SplMemWriteRespType& d,
      const std::string& name) {
  }

  inline friend std::ostream& operator<<(std::ostream& os,
      const SplMemWriteRespType& d) {
    os << "ack: " << d.ack << std::endl;
    os << "io_unit_id: " << d.io_unit_id << std::endl;
    return os;
  }

  inline bool operator==(const SplMemWriteRespType& rhs) {
    bool result = true;
    result = result && (ack == rhs.ack);
    result = result && (io_unit_id == rhs.io_unit_id);
    result = result && (tag == rhs.tag);
    return result;
  }

};

template <typename T, size_t REQ_BUF_SIZE=16, size_t MAX_BURST_COUNT=256, size_t UTYPE_REQ_BUF_SIZE=REQ_BUF_SIZE>
struct LoadUnitParams {
  typedef T type;
  enum {
    cl_request_buffer_size = REQ_BUF_SIZE,
    utype_max_request_burst_count = MAX_BURST_COUNT,
    utype_request_buffer_size = UTYPE_REQ_BUF_SIZE
  };
};

template <typename T, typename UTAG, size_t UTYPE_REQ_BUF_SIZE>
struct LoadUnitSingleReqParams {
  typedef T type;
  typedef UTAG UserTagType;
  enum {
    utype_request_buffer_size = UTYPE_REQ_BUF_SIZE
  };
};

template <typename T>
struct StoreUnitParams {
  typedef T type;
};

template <typename T, typename UTAG, size_t UTYPE_REQ_BUF_SIZE>
struct StoreUnitSingleReqParams {
  typedef T type;
  typedef UTAG UserTagType;
  enum {
    utype_request_buffer_size = UTYPE_REQ_BUF_SIZE
  };
};

template <typename T, typename UTAG, size_t NUM_OF_SETS, size_t NUM_OF_WAYS, size_t MSRH_SIZE, size_t SAME_ADDR_REQ_BUF_SIZE>
struct LoadStoreUnitSingleReqParams {
  typedef T type;
  typedef UTAG UserTagType;
  enum {
    num_of_sets = NUM_OF_SETS,
    num_of_ways = NUM_OF_WAYS,
    msrh_size = MSRH_SIZE,
    same_addr_req_buf_size = SAME_ADDR_REQ_BUF_SIZE
  };
};

struct EmptyStruct {};

#define DEFINE_TYPE(___OrigType,___NewType,___BitCnt) typedef ___OrigType ___NewType; const size_t ___NewType##BitCnt = ___BitCnt

template <typename T>
struct BitCntUtil {
  size_t getBitCnt() {
    return T::getBitCnt();
  }
};



#endif //__ACC_TYPES__
