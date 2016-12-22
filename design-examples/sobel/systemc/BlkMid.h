// See LICENSE for license details.

/*[[[cog
     import cog
     from cog_acctempl import *
     from dut_params import *
     if "ty" not in globals():
       lst = cog.previous.lstrip('/').rstrip('\n').split('=')
       assert( lst[0]=="ty")
       assert( len(lst)==2)
       global ty
       ty = lst[1]
     cog.outl( "//ty=" + ty)
     ut = dut.usertypes[ty]
  ]]]*/
//ty=BlkMid
//[[[end]]] (checksum: 18a5d7b64e018cebce91cf9af30d3abc)
/*[[[cog
     cog.outl("#ifndef %s_H_" % ty)
     cog.outl("#define %s_H_" % ty)
  ]]]*/
#ifndef BlkMid_H_
#define BlkMid_H_
//[[[end]]] (checksum: b9cbb7b53a9f25abd69430c09cc7d6e6)

#ifndef __SYNTHESIS__
#include <cstddef>
#include <cassert>
#endif

/*[[[cog
     cog.outl("class %s {" % ty)
  ]]]*/
class BlkMid {
//[[[end]]] (checksum: 586e49bbfd96d140252c48e7e2502b47)
public:
  /*[[[cog
       for field in ut.fields:
         cog.outl( field.declaration)
    ]]]*/
  short data[64];
  //[[[end]]] (checksum: 18e6057facc215e1b9ecb9d320fcb506)

  /*[[[cog
       cog.outl("enum { BitCnt = %d };" % ut.bitwidth)
    ]]]*/
  enum { BitCnt = 1024 };
  //[[[end]]] (checksum: 82d2f6dfad1fae7eee99a7f3ff03f3e9)

  static size_t getBitCnt() {
    /*[[[cog
         cog.outl("assert(sizeof(%s) == (size_t) BitCnt/8);" % ty)
      ]]]*/
    assert(sizeof(BlkMid) == (size_t) BitCnt/8);
    //[[[end]]] (checksum: bdee74a691a4548f323f6d2975f6c2c7)
    assert( 0 == (size_t) BitCnt%8);
    return BitCnt;
  }
  static size_t numberOfFields() {
    /*[[[cog
         cog.outl("return %d;" % ut.numberOfFields)
      ]]]*/
    return 64;
    //[[[end]]] (checksum: 240c70ce30899a0d6f6b12692226a63a)
  }
  static size_t fieldWidth( size_t index) {
    /*[[[cog
         sum = 0
         for field in ut.fields:
            cog.outl("if ( %d <= index && index < %d) {" % (sum,sum+field.numberOfFields))
            cog.outl( field.fieldWidth( sum))
            cog.outl("}")
            sum += field.numberOfFields
      ]]]*/
    if ( 0 <= index && index < 64) {
      return 16;
    }
    //[[[end]]] (checksum: f5176f18f0263c18d624d2ace15d3277)
    return 0;
  }
  void putField(size_t index, UInt64 d) {
    /*[[[cog
         sum = 0
         for field in ut.fields:
            cog.outl("if ( %d <= index && index < %d) {" % (sum,sum+field.numberOfFields))
            cog.outl( field.putField( sum))
            cog.outl("}")
            sum += field.numberOfFields
      ]]]*/
    if ( 0 <= index && index < 64) {
      data[index-0] = d;
    }
    //[[[end]]] (checksum: 0133ceac191f2b6f57035a00e19d5e01)
  }
  UInt64 getField(size_t index) const {
    /*[[[cog
         sum = 0
         for field in ut.fields:
            cog.outl("if ( %d <= index && index < %d) {" % (sum,sum+field.numberOfFields))
            cog.outl( field.getField( sum))
            cog.outl("}")
            sum += field.numberOfFields
      ]]]*/
    if ( 0 <= index && index < 64) {
      return data[index-0];
    }
    //[[[end]]] (checksum: 0da040286536b233d568d8daa61c22f0)
    return 0;
  }

#if !defined(__AAL_USER__) && !defined(USE_SOFTWARE)
  /*[[[cog
       cog.outl("inline friend void sc_trace(sc_trace_file* tf, const %s& d, const std::string& name) {" % ty)
    ]]]*/
  inline friend void sc_trace(sc_trace_file* tf, const BlkMid& d, const std::string& name) {
  //[[[end]]] (checksum: 1ef581e9281252c7613cf0e05b4f56da)
  }
#endif

  /*[[[cog
       cog.outl("inline friend std::ostream& operator<<(std::ostream& os, const %s& d) {" % ty)
       cog.outl("  os << \"<%s>\";" % ty)
    ]]]*/
  inline friend std::ostream& operator<<(std::ostream& os, const BlkMid& d) {
    os << "<BlkMid>";
  //[[[end]]] (checksum: b4bcdad37e07628f5ca72364c9f85c16)
    return os;
  }

  /*[[[cog
       cog.outl("inline bool operator==(const %s& rhs) const {" % ty)
    ]]]*/
  inline bool operator==(const BlkMid& rhs) const {
  //[[[end]]] (checksum: 1d94ec522d042ae3e290811773e62b90)
    bool result = true;
    for( unsigned int i=0; i<numberOfFields(); ++i) {
      result = result && (getField(i) == rhs.getField(i));
    }
    return result;
  }

};

#endif
