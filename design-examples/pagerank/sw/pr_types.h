// See LICENSE for license details.

#ifndef __PRTYPES_H__
#define __PRTYPES_H__

#ifndef __SYNTHESIS__
#include <iostream>
#include <string>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#endif
using std::endl ;
using std::cout ;
using std::string ;

//#include "systemc.h"
//#include "types.h"
#include "reinterpret_utils.h"

#define ITER_COUNT 1
#define BETA 0.85

typedef unsigned long long AddrType;

#ifdef USE_HLS
#include "ctos_fx.h"
typedef UREAL_VAL_TYPE(32,1) PRreal;
//typedef ctos_sc_dt::sc_ufixed<32, 1, sc_dt::SC_RND, sc_dt::SC_SAT> PRreal;

#elif __AAL_USER__
//typedef REAL_VAL_TYPE(32,1) PRreal;
typedef sc_dt::sc_ufixed<32,1> PRreal;
//typedef float PRreal;
#else
typedef float PRreal;
#endif
#define PR_TYPE_WIDTH 32

// id from hld_defines.h as a string
#define PR_AFU_ID "ce53_7515_9d3a_cabc94ec_b552_08f1_5b01"



typedef unsigned int VtxId;
typedef unsigned int EdgeId;
typedef unsigned int VtxDegree;


#ifndef _SYNTHESIS_
#define FABS fabs
#else
#define FABS if(0) fabs
#endif



#define DEALLOC_ARR(__ptr) {\
    if (__ptr) 					\
    delete [] __ptr ;}


struct PageRankType {
  enum FieldWidths {
    prWidth = PR_TYPE_WIDTH
  };

private:
  unsigned pr : prWidth;
public:

  PageRankType() : pr(0) {}
  PageRankType(PRreal pr_in)
  {
    setPr(pr_in);
  }

  void setPr(PRreal pr_in) {
    pr = cast_utils::reinterpret_type<prWidth, unsigned>(pr_in);
    //std::cout << "SET PR " << pr_in << " " << pr << endl;
  }

  PRreal getPr() {
    PRreal result = cast_utils::reinterpret_type<prWidth, PRreal>(pr);
    return result;
  }

  enum {
    BitCnt = prWidth
  };

  static size_t getBitCnt() {
    assert(sizeof(PageRankType)==(size_t)prWidth/8);
    return (size_t)prWidth;
  }

  static size_t numberOfFields() {
    return 1;
  }

  static size_t fieldWidth(size_t index) {
    switch(index) {
    case 0:
      return 32;
    default:
      assert(0);
    }
    return 0;
  }
  void putField(size_t index, UInt64 d) {
    switch(index) {
    case 0: {
      pr = d;
      break;
    }
    default:
      assert(0);
    }
  }

  UInt64 getField(size_t index) const {
    switch(index) {
    case 0:
      return pr;
    default:
      assert(0);
    }
    return 0;
  }

#ifndef __AAL_USER__
  inline friend void sc_trace(sc_trace_file* tf, const PageRankType& d,
      const std::string& name) {
  }
#endif

  inline friend std::ostream& operator<<(std::ostream& os,
      const PageRankType& d) {
    os << "pr: " << d.pr << std::endl;
    return os;
  }

  inline bool operator==(const PageRankType& rhs) const {
    bool result = true;
    result = result && (pr == rhs.pr);
    return result;
  }
};


struct VertexDataSW {
  PRreal oneOverVtxDegrees;
  EdgeId ovdOffset;
  EdgeId ovdSize;
};

struct VertexData {
  enum FieldWidths {
    oneOverVtxDegreesWidth = PR_TYPE_WIDTH,
    ovdOffsetWidth = 32,
    ovdSizeWidth = 32,
    dummyWidth = 32,
    BitCnt = 128
  };

  typedef PRreal oneOverVtxDegreesType;
  typedef EdgeId ovdOffsetType;
  typedef EdgeId ovdSizeWidthType;
  typedef EdgeId dummyType;

private:
  unsigned oneOverVtxDegrees : oneOverVtxDegreesWidth;
public:
  unsigned ovdOffset : ovdOffsetWidth;
  unsigned ovdSize : ovdSizeWidth;
  unsigned dummy : dummyWidth;

  void setOneOverVtxDegree(PRreal v) {
    oneOverVtxDegrees = cast_utils::reinterpret_type<oneOverVtxDegreesWidth, unsigned>(v);
  }

  PRreal getOneOverVtxDegree() {
    return cast_utils::reinterpret_type<oneOverVtxDegreesWidth, PRreal>(oneOverVtxDegrees);
  }

  static size_t getBitCnt() {
    assert(sizeof(VertexData)==(size_t)BitCnt/8);
    return BitCnt;
  }

  static size_t numberOfFields() {
    return 4;
  }

  static size_t fieldWidth(size_t index) {
    switch(index) {
    case 0:
      return oneOverVtxDegreesWidth;
    case 1:
      return ovdOffsetWidth;
    case 2:
      return ovdSizeWidth;
    case 3:
      return dummyWidth;
    default:
      assert(0);
    }
    return 0;
  }
  void putField(size_t index, UInt64 d) {
    switch(index) {
    case 0:
      oneOverVtxDegrees = d;
      break;
    case 1:
      ovdOffset = d;
      break;
    case 2:
      ovdSize = d;
      break;
    case 3:
      dummy = d;
      break;
    default:
      assert(0);
    }
  }

  UInt64 getField(size_t index) {
    switch(index) {
    case 0:
      return oneOverVtxDegrees;
    case 1:
      return ovdOffset;
    case 2:
      return ovdSize;
    case 3:
      return dummy;
    default:
      assert(0);
    }
    return 0;
  }
#ifndef __AAL_USER__
  inline friend void sc_trace(sc_trace_file* tf, const VertexData& d,
      const std::string& name) {
  }
#endif

  inline friend std::ostream& operator<<(std::ostream& os,
      const VertexData& d) {
    os << "oneOverVtxDegrees: " << d.oneOverVtxDegrees << std::endl;
    os << "ovdOffset: " << d.ovdOffset << std::endl;
    os << "ovdSize: " << d.ovdSize << std::endl;
    os << "dummy: " << d.dummy << std::endl;
    return os;
  }

  inline bool operator==(const VertexData& rhs) const {
    bool result = true;
    result = result && (oneOverVtxDegrees == rhs.oneOverVtxDegrees);
    result = result && (ovdOffset == rhs.ovdOffset);
    result = result && (ovdSize == rhs.ovdSize);
    result = result && (dummy == rhs.dummy);
    return result;
  }
};


struct VertexIdType {
  VtxId vid;

  VertexIdType() {}

  VertexIdType(VtxId id) :
    vid(id) {}

  enum {
    BitCnt = 32
  };

  static size_t getBitCnt() {
    return BitCnt;
  }

  static size_t numberOfFields() {
    return 1;
  }

  static size_t fieldWidth(size_t index) {
    switch(index) {
    case 0:
      return 32;
    default:
      assert(0);
    }
    return 0;
  }
  void putField(size_t index, UInt64 d) {
    switch(index) {
    case 0:
      vid = (VtxId)d;
      break;
    default:
      assert(0);
    }
  }

  UInt64 getField(size_t index) const {
    switch(index) {
    case 0:
      return vid;
    default:
      assert(0);
    }
    return 0;
  }

#ifndef __AAL_USER__
  inline friend void sc_trace(sc_trace_file* tf, const VertexIdType& d,
      const std::string& name) {
  }
#endif

  inline friend std::ostream& operator<<(std::ostream& os,
      const VertexIdType& d) {
    os << "vid: " << d.vid << std::endl;
    return os;
  }

  inline bool operator==(const VertexIdType& rhs) const {
    bool result = true;
    result = result && (vid == rhs.vid);
    return result;
  }
};


struct Config {
  // addr offsets
private:
  unsigned long long aVD : 64;
  unsigned int aReadPR   : 32;
  unsigned int aWritePR  : 32;
  unsigned int aVID      : 32;
  unsigned int RANKOFFSET: 32;

public:
  // number of vertices
  unsigned int vtxCnt    : 32;
  // number of iterations
  unsigned int iterCount : 32 ;
  // number of iterations
  unsigned int vtxOffset : 32 ;
  // number of iterations
  unsigned int edgeCnt : 32 ;

public:
  Config() {
    aVD = aReadPR = aWritePR = aVID = RANKOFFSET = vtxCnt = iterCount = vtxOffset = edgeCnt = 0;
  }
  void copy(Config &from) {
    aVD = from.aVD;
    aReadPR = from.aReadPR;
    aWritePR = from.aWritePR;
    aVID = from.aVID;
    RANKOFFSET = from.RANKOFFSET;
    vtxCnt = from.vtxCnt;
    iterCount = from.iterCount;
    vtxOffset = from.vtxOffset;
    edgeCnt = from.edgeCnt;
  }

  AddrType getVDAddr(unsigned int vid) const {
    return aVD + (VertexData::getBitCnt()/8)*vid;
  }
  AddrType getReadPRAddr(unsigned int vid) const {
    return aVD + aReadPR + (PageRankType::getBitCnt()/8)*vid;
  }

  AddrType getWritePRAddr(unsigned int vid) const {
    return aVD + aWritePR + (PageRankType::getBitCnt()/8)*vid;
  }
  AddrType getVIDAddr(unsigned int offset) const {
    return aVD + aVID + (VertexIdType::getBitCnt()/8)*offset;
  }

  PRreal getRankOffset() const {
    return cast_utils::reinterpret_type<32, PRreal>(RANKOFFSET);
  }
  void setRankOffset(PRreal pr) {
    RANKOFFSET = cast_utils::reinterpret_type<32, unsigned int>(pr);
  }

  void swapReadWritePR() {
    unsigned int tmp = aReadPR;
    aReadPR = aWritePR;
    aWritePR = tmp;
  }

  void setVDAddr(AddrType addr) {
    aVD = addr;
  }
  void setReadPRAddr(AddrType addr) {
    aReadPR = addr;
  }

  void setWritePRAddr(AddrType addr) {
    aWritePR = addr;
  }
  void setVIDAddr(AddrType addr) {
    aVID = addr;
  }

#ifndef __AAL_USER__
  inline friend void sc_trace(sc_trace_file* tf, const Config& d,
      const std::string& name) {
  }
#endif

  inline friend std::ostream& operator<<(std::ostream& os, const Config& d) {
    os << "aVD: " << d.aVD << std::endl;
    os << "aReadPR: " << d.aReadPR << std::endl;
    os << "aWritePR: " << d.aWritePR << std::endl;
    os << "aVID: " << d.aVID << std::endl;
    os << "vtxOffset: " << d.vtxOffset << std::endl;
    os << "vtxCnt: " << d.vtxCnt << std::endl;
    os << "RANKOFFSET: " << d.RANKOFFSET << std::endl;
    return os;
  }

  inline bool operator==(const Config& rhs) const {
    bool result = true;
    result = result && (aVD == rhs.aVD);
    result = result && (aReadPR == rhs.aReadPR);
    result = result && (aWritePR == rhs.aWritePR);
    result = result && (aVID == rhs.aVID);
    result = result && (vtxCnt == rhs.vtxCnt);
    result = result && (RANKOFFSET == rhs.RANKOFFSET);
    result = result && (iterCount == rhs.iterCount);
    result = result && (vtxOffset == rhs.vtxOffset);
    result = result && (edgeCnt == rhs.edgeCnt);
    return result;
  }
};

#endif
