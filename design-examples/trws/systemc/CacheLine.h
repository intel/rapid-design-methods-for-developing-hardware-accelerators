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
//ty=CacheLine
//[[[end]]] (checksum: 83c2abd91c3bc28abca443638a359778)
/*[[[cog
     cog.outl("#ifndef %s_H_" % ty)
     cog.outl("#define %s_H_" % ty)
  ]]]*/
#ifndef CacheLine_H_
#define CacheLine_H_
//[[[end]]] (checksum: ff5a1bc69d990f6d90fbb9f266b06299)

#ifndef __SYNTHESIS__
#include <cstddef>
#include <cassert>
#endif

/*[[[cog
     cog.outl("class %s {" % ty)
  ]]]*/
class CacheLine {
//[[[end]]] (checksum: a5bfab3cfa50fb1bb039b116ec8db585)
public:
  /*[[[cog
       for field in ut.fields:
         cog.outl( field.declaration)
    ]]]*/
  short words[32];
  //[[[end]]] (checksum: f4ed527d77f281d8b805fb91b5c17619)

  /*[[[cog
       if len(ut.fields) == 1 and type(ut.fields[0]) is ArrayField:
         cog.outl( "enum { ArrayLength = %d };" % ut.fields[0].count)
         cog.outl( "typedef %s ElementType;" % ut.fields[0].ty.ty)
    ]]]*/
  enum { ArrayLength = 32 };
  typedef short ElementType;
  //[[[end]]] (checksum: df8a7d9f6f66bc6fe446a632570926b6)

  /*[[[cog
       cog.outl("enum { BitCnt = %d };" % ut.bitwidth)
    ]]]*/
  enum { BitCnt = 512 };
  //[[[end]]] (checksum: 37159abeda6c5f75899fb314e1ed078c)

  static size_t getBitCnt() {
    /*[[[cog
         cog.outl("assert(sizeof(%s) == (size_t) BitCnt/8);" % ty)
      ]]]*/
    assert(sizeof(CacheLine) == (size_t) BitCnt/8);
    //[[[end]]] (checksum: 876cb2e24897c7e2cfec92b111262852)
    assert( 0 == (size_t) BitCnt%8);
    return BitCnt;
  }
  static size_t numberOfFields() {
    /*[[[cog
         cog.outl("return %d;" % ut.numberOfFields)
      ]]]*/
    return 32;
    //[[[end]]] (checksum: 163e9ad09a380163fbc65c841051d031)
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
    if ( 0 <= index && index < 32) {
      return 16;
    }
    //[[[end]]] (checksum: c5db7f6707d62f6f22860adc80533077)
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
    if ( 0 <= index && index < 32) {
      words[index-0] = d;
    }
    //[[[end]]] (checksum: 498ba8aec050a1441743cd24a2e15076)
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
    if ( 0 <= index && index < 32) {
      return words[index-0];
    }
    //[[[end]]] (checksum: 2cf5fc24e001cf68c666e6425f341976)
    return 0;
  }

#if !defined(__AAL_USER__) && !defined(USE_SOFTWARE)
  /*[[[cog
       cog.outl("inline friend void sc_trace(sc_trace_file* tf, const %s& d, const std::string& name) {" % ty)
    ]]]*/
  inline friend void sc_trace(sc_trace_file* tf, const CacheLine& d, const std::string& name) {
  //[[[end]]] (checksum: 831cd4fced8f6f9d1af3f7b67184262e)
  }
#endif

  /*[[[cog
       cog.outl("inline friend std::ostream& operator<<(std::ostream& os, const %s& d) {" % ty)
       cog.outl("  os << \"<%s>\";" % ty)
    ]]]*/
  inline friend std::ostream& operator<<(std::ostream& os, const CacheLine& d) {
    os << "<CacheLine>";
  //[[[end]]] (checksum: 720cf409d0cc7b5eb8f2c444297d0e84)
    return os;
  }

  /*[[[cog
       cog.outl("inline bool operator==(const %s& rhs) const {" % ty)
    ]]]*/
  inline bool operator==(const CacheLine& rhs) const {
  //[[[end]]] (checksum: cf4783549073f665518590a4ec88dbc4)
    bool result = true;
    for( unsigned int i=0; i<numberOfFields(); ++i) {
      result = result && (getField(i) == rhs.getField(i));
    }
    return result;
  }

};

#endif
