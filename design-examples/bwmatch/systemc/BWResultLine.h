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
//ty=BWResultLine
//[[[end]]] (checksum: 7b11b04d39843b5bb841c2544be42869)
/*[[[cog
     cog.outl("#ifndef %s_H_" % ty)
     cog.outl("#define %s_H_" % ty)
  ]]]*/
#ifndef BWResultLine_H_
#define BWResultLine_H_
//[[[end]]] (checksum: efa8f6fe1eb6b5f99b00615cc9fa5799)

#ifndef __SYNTHESIS__
#include <cstddef>
#include <cassert>
#endif

/*[[[cog
     cog.outl("class %s {" % ty)
  ]]]*/
class BWResultLine {
//[[[end]]] (checksum: ae52a61d8c7f962cb5dba082108894ab)
public:
  /*[[[cog
       for field in ut.fields:
         cog.outl( field.declaration)
    ]]]*/
  BWResult line[8];
  //[[[end]]] (checksum: cb740b14f14afa640392b16fc86e21f3)

  /*[[[cog
       cog.outl("enum { BitCnt = %d };" % ut.bitwidth)
    ]]]*/
  enum { BitCnt = 512 };
  //[[[end]]] (checksum: 37159abeda6c5f75899fb314e1ed078c)

  static size_t getBitCnt() {
    /*[[[cog
         cog.outl("assert(sizeof(%s) == (size_t) BitCnt/8);" % ty)
      ]]]*/
    assert(sizeof(BWResultLine) == (size_t) BitCnt/8);
    //[[[end]]] (checksum: f260c589d4e6924544df194c55e48741)
    assert( 0 == (size_t) BitCnt%8);
    return BitCnt;
  }
  static size_t numberOfFields() {
    /*[[[cog
         cog.outl("return %d;" % ut.numberOfFields)
      ]]]*/
    return 16;
    //[[[end]]] (checksum: b25f9945c9aa3961f46fba561fa82ca9)
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
    if ( 0 <= index && index < 16) {
      return BWResult::fieldWidth( (index-0) % 2);
    }
    //[[[end]]] (checksum: b9cfc793559a0b2e854e7e9be35fd97d)
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
    if ( 0 <= index && index < 16) {
      line[(index-0)/2].putField( (index-0) % 2, d);
    }
    //[[[end]]] (checksum: fcb008d8205a70969d921e2da0756ab5)
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
    if ( 0 <= index && index < 16) {
      return line[(index-0)/2].getField( (index-0) % 2);
    }
    //[[[end]]] (checksum: bfcc0e38ed3611b6f0862659e9aee7eb)
    return 0;
  }

#if !defined(__AAL_USER__) && !defined(USE_SOFTWARE)
  /*[[[cog
       cog.outl("inline friend void sc_trace(sc_trace_file* tf, const %s& d, const std::string& name) {" % ty)
    ]]]*/
  inline friend void sc_trace(sc_trace_file* tf, const BWResultLine& d, const std::string& name) {
  //[[[end]]] (checksum: 845a9f7e15aa0011b60dbee926a8e90f)
  }
#endif

  /*[[[cog
       cog.outl("inline friend std::ostream& operator<<(std::ostream& os, const %s& d) {" % ty)
       cog.outl("  os << \"<%s>\";" % ty)
    ]]]*/
  inline friend std::ostream& operator<<(std::ostream& os, const BWResultLine& d) {
    os << "<BWResultLine>";
  //[[[end]]] (checksum: 84efbadc24df139b29b169317b414175)
    return os;
  }

  /*[[[cog
       cog.outl("inline bool operator==(const %s& rhs) const {" % ty)
    ]]]*/
  inline bool operator==(const BWResultLine& rhs) const {
  //[[[end]]] (checksum: 800ef366c6bb98043518f0a7b3d70886)
    bool result = true;
    for( unsigned int i=0; i<numberOfFields(); ++i) {
      result = result && (getField(i) == rhs.getField(i));
    }
    return result;
  }

};

#endif
