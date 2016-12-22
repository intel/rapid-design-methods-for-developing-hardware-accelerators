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
//ty=Blk
//[[[end]]] (checksum: 9eb1f81865996081a2b967da18bdb2e3)
/*[[[cog
     cog.outl("#ifndef %s_H_" % ty)
     cog.outl("#define %s_H_" % ty)
  ]]]*/
#ifndef Blk_H_
#define Blk_H_
//[[[end]]] (checksum: 480ec3e82dc709bb218f26f47e0d2e40)

#ifndef __SYNTHESIS__
#include <cstddef>
#include <cassert>
#endif

/*[[[cog
     cog.outl("class %s {" % ty)
  ]]]*/
class Blk {
//[[[end]]] (checksum: e1a5420d28d9af6eda54502b70510b3a)
public:
  /*[[[cog
       for field in ut.fields:
         cog.outl( field.declaration)
    ]]]*/
  unsigned int words[8];
  //[[[end]]] (checksum: cdd7d5786639aa5ac05497fbb9d7159b)

  /*[[[cog
       cog.outl("enum { BitCnt = %d };" % ut.bitwidth)
    ]]]*/
  enum { BitCnt = 256 };
  //[[[end]]] (checksum: a641fbc923747c10e4cf6e3f11569112)

  static size_t getBitCnt() {
    /*[[[cog
         cog.outl("assert(sizeof(%s) == (size_t) BitCnt/8);" % ty)
      ]]]*/
    assert(sizeof(Blk) == (size_t) BitCnt/8);
    //[[[end]]] (checksum: 6bd1e354457ba98b3679d99a159176fd)
    assert( 0 == (size_t) BitCnt%8);
    return BitCnt;
  }
  static size_t numberOfFields() {
    /*[[[cog
         cog.outl("return %d;" % ut.numberOfFields)
      ]]]*/
    return 8;
    //[[[end]]] (checksum: 314c347cd02990f9b8506ab115315c18)
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
    if ( 0 <= index && index < 8) {
      return 32;
    }
    //[[[end]]] (checksum: 2d3e3b03557ee4a101efe1001137b1b4)
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
    if ( 0 <= index && index < 8) {
      words[index-0] = d;
    }
    //[[[end]]] (checksum: ab11b9b32dcfc398fc04749e7e1360e1)
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
    if ( 0 <= index && index < 8) {
      return words[index-0];
    }
    //[[[end]]] (checksum: 16a9cfe200e7780c0afe0add24e543bf)
    return 0;
  }

#if !defined(__AAL_USER__) && !defined(USE_SOFTWARE)
  /*[[[cog
       cog.outl("inline friend void sc_trace(sc_trace_file* tf, const %s& d, const std::string& name) {" % ty)
    ]]]*/
  inline friend void sc_trace(sc_trace_file* tf, const Blk& d, const std::string& name) {
  //[[[end]]] (checksum: ca11ad44286df3fb1139ff9cca688644)
  }
#endif

  /*[[[cog
       cog.outl("inline friend std::ostream& operator<<(std::ostream& os, const %s& d) {" % ty)
       cog.outl("  os << \"<%s>\";" % ty)
    ]]]*/
  inline friend std::ostream& operator<<(std::ostream& os, const Blk& d) {
    os << "<Blk>";
  //[[[end]]] (checksum: 95a1a578ec863009afc2c57a09a72229)
    return os;
  }

  /*[[[cog
       cog.outl("inline bool operator==(const %s& rhs) const {" % ty)
    ]]]*/
  inline bool operator==(const Blk& rhs) const {
  //[[[end]]] (checksum: 38b5e8198eb7c6dc3ca55f0a0b2871a1)
    bool result = true;
    for( unsigned int i=0; i<numberOfFields(); ++i) {
      result = result && (getField(i) == rhs.getField(i));
    }
    return result;
  }

};

#endif
