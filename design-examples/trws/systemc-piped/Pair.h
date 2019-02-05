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
//ty=Pair
//[[[end]]] (checksum: c7ca9f4048f2ea760d3eb021cfd5cc15)
/*[[[cog
     cog.outl("#ifndef %s_H_" % ty)
     cog.outl("#define %s_H_" % ty)
  ]]]*/
#ifndef Pair_H_
#define Pair_H_
//[[[end]]] (checksum: 0a7828b52bd62cb78263fd219d4629ff)

#ifndef __SYNTHESIS__
#include <cstddef>
#include <cassert>
#endif

/*[[[cog
     cog.outl("class %s {" % ty)
  ]]]*/
#pragma ctos monolithic
class Pair {
//[[[end]]] (checksum: 2506a07e6c117da7d38222e20ccbe474)
public:
  /*[[[cog
       for field in ut.fields:
         cog.outl( field.declaration)
    ]]]*/
  short a;
  unsigned short e;
  //[[[end]]] (checksum: e14abc4c3ef440fa8876c6526f603e4e)

  /*[[[cog
       if len(ut.fields) == 1 and type(ut.fields[0]) is ArrayField:
         cog.outl( "enum { ArrayLength = %d };" % ut.fields[0].count)
         cog.outl( "typedef %s ElementType;" % ut.fields[0].ty.ty)
    ]]]*/
  //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)

  /*[[[cog
       cog.outl("enum { BitCnt = %d };" % ut.bitwidth)
    ]]]*/
  enum { BitCnt = 32 };
  //[[[end]]] (checksum: bb636e1fc432e8e460875ab4d601f55b)

  static size_t getBitCnt() {
    /*[[[cog
         cog.outl("assert(sizeof(%s) == (size_t) BitCnt/8);" % ty)
      ]]]*/
    assert(sizeof(Pair) == (size_t) BitCnt/8);
    //[[[end]]] (checksum: 3a707da7a5ac6d20476916bc82fbcefc)
    assert( 0 == (size_t) BitCnt%8);
    return BitCnt;
  }
  static size_t numberOfFields() {
    /*[[[cog
         cog.outl("return %d;" % ut.numberOfFields)
      ]]]*/
    return 2;
    //[[[end]]] (checksum: 6cc53858759914f3431a5d5f3cef2835)
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
    if ( 0 <= index && index < 1) {
      return 16;
    }
    if ( 1 <= index && index < 2) {
      return 16;
    }
    //[[[end]]] (checksum: 795c291cb8746da7e83da85873737a6e)
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
    if ( 0 <= index && index < 1) {
      a = d;
    }
    if ( 1 <= index && index < 2) {
      e = d;
    }
    //[[[end]]] (checksum: 309b8dc52cc7ce13cf7cb1cabb485e3c)
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
    if ( 0 <= index && index < 1) {
      return a;
    }
    if ( 1 <= index && index < 2) {
      return e;
    }
    //[[[end]]] (checksum: 9979956a54f30564c41ced2bdb7149d8)
    return 0;
  }

#if !defined(__AAL_USER__) && !defined(USE_SOFTWARE)
  /*[[[cog
       cog.outl("inline friend void sc_trace(sc_trace_file* tf, const %s& d, const std::string& name) {" % ty)
    ]]]*/
  inline friend void sc_trace(sc_trace_file* tf, const Pair& d, const std::string& name) {
  //[[[end]]] (checksum: 15834f3cb3c22b745a02260d1360dc06)
  }
#endif

  /*[[[cog
       cog.outl("inline friend std::ostream& operator<<(std::ostream& os, const %s& d) {" % ty)
       cog.outl("  os << \"<%s>\";" % ty)
    ]]]*/
  inline friend std::ostream& operator<<(std::ostream& os, const Pair& d) {
    os << "<Pair>";
  //[[[end]]] (checksum: 4a2c157a16f9bec54226f58ae8d07296)
    return os;
  }

  /*[[[cog
       cog.outl("inline bool operator==(const %s& rhs) const {" % ty)
    ]]]*/
  inline bool operator==(const Pair& rhs) const {
  //[[[end]]] (checksum: 0eab33fad860e64fae215feb509eb9d1)
    bool result = true;
    for( unsigned int i=0; i<numberOfFields(); ++i) {
      result = result && (getField(i) == rhs.getField(i));
    }
    return result;
  }

};

#endif
