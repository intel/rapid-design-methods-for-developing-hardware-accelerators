// See LICENSE for license details.
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
//ty=Node
//[[[end]]] (checksum: 4f11fda5ef69d65811f5300e1a4e1999)
/*[[[cog
     cog.outl("#ifndef %s_H_" % ty)
     cog.outl("#define %s_H_" % ty)
  ]]]*/
#ifndef Node_H_
#define Node_H_
//[[[end]]] (checksum: 6676666d1672f8be8725cc5c137574c1)

#ifndef __SYNTHESIS__
#include <cstddef>
#include <cassert>
#endif

/*[[[cog
     cog.outl("class %s {" % ty)
  ]]]*/
class Node {
//[[[end]]] (checksum: 4de14371568bc64fe4efbaf77385f659)
public:
  /*[[[cog
       for field in ut.fields:
         cog.outl( field.declaration)
    ]]]*/
  unsigned long long next_offset;
  unsigned long long val;
  //[[[end]]] (checksum: 9aaaadd4c6d5c0f6bb3b59ac8a01ef54)

  /*[[[cog
       cog.outl("enum { BitCnt = %d };" % ut.bitwidth)
    ]]]*/
  enum { BitCnt = 128 };
  //[[[end]]] (checksum: a5b768f10ab67444eff67741636e53f7)

  static size_t getBitCnt() {
    /*[[[cog
         cog.outl("assert(sizeof(%s) == (size_t) BitCnt/8);" % ty)
      ]]]*/
    assert(sizeof(Node) == (size_t) BitCnt/8);
    //[[[end]]] (checksum: 8a7c0d9b71061bc2e3ff9c0a31817041)
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
      return 64;
    }
    if ( 1 <= index && index < 2) {
      return 64;
    }
    //[[[end]]] (checksum: f1811c187583ca4d8fc39a86df2036b2)
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
      next_offset = d;
    }
    if ( 1 <= index && index < 2) {
      val = d;
    }
    //[[[end]]] (checksum: 271232c4b19a6c90d4718d186173ceff)
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
      return next_offset;
    }
    if ( 1 <= index && index < 2) {
      return val;
    }
    //[[[end]]] (checksum: d52c1555df5eff1a869b83dcaecca127)
    return 0;
  }

#if !defined(__AAL_USER__) && !defined(USE_SOFTWARE)
  /*[[[cog
       cog.outl("inline friend void sc_trace(sc_trace_file* tf, const %s& d, const std::string& name) {" % ty)
    ]]]*/
  inline friend void sc_trace(sc_trace_file* tf, const Node& d, const std::string& name) {
  //[[[end]]] (checksum: 35d921f52889b232448db20a5a847bac)
  }
#endif

  /*[[[cog
       cog.outl("inline friend std::ostream& operator<<(std::ostream& os, const %s& d) {" % ty)
       cog.outl("  os << \"<%s>\";" % ty)
    ]]]*/
  inline friend std::ostream& operator<<(std::ostream& os, const Node& d) {
    os << "<Node>";
  //[[[end]]] (checksum: 2458c1038df7e907eeb2a1c2d113c812)
    return os;
  }

  /*[[[cog
       cog.outl("inline bool operator==(const %s& rhs) const {" % ty)
    ]]]*/
  inline bool operator==(const Node& rhs) const {
  //[[[end]]] (checksum: f182cea338c3a889ac8b7d0e83fc99ef)
    bool result = true;
    for( unsigned int i=0; i<numberOfFields(); ++i) {
      result = result && (getField(i) == rhs.getField(i));
    }
    return result;
  }

};

#endif
