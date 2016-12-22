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
//ty=BlkOut
//[[[end]]] (checksum: 3a6b8240cb405f87ab49a99a049fa8bb)
/*[[[cog
     cog.outl("#ifndef %s_H_" % ty)
     cog.outl("#define %s_H_" % ty)
  ]]]*/
#ifndef BlkOut_H_
#define BlkOut_H_
//[[[end]]] (checksum: 0f430546a696271f85fc82959c3df3c6)

#ifndef __SYNTHESIS__
#include <cstddef>
#include <cassert>
#endif

/*[[[cog
     cog.outl("class %s {" % ty)
  ]]]*/
class BlkOut {
//[[[end]]] (checksum: e3b70eea1d7090569675c55e86fda80e)
public:
  /*[[[cog
       for field in ut.fields:
         cog.outl( field.declaration)
    ]]]*/
  char data[32];
  //[[[end]]] (checksum: 988f71d42ff5994a78dea4a3495138cb)

  /*[[[cog
       cog.outl("enum { BitCnt = %d };" % ut.bitwidth)
    ]]]*/
  enum { BitCnt = 256 };
  //[[[end]]] (checksum: a641fbc923747c10e4cf6e3f11569112)

  static size_t getBitCnt() {
    /*[[[cog
         cog.outl("assert(sizeof(%s) == (size_t) BitCnt/8);" % ty)
      ]]]*/
    assert(sizeof(BlkOut) == (size_t) BitCnt/8);
    //[[[end]]] (checksum: 6de7bf5c75ff38fab12bbe1bfd7e359d)
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
      return 8;
    }
    //[[[end]]] (checksum: c8c6aa6074ee6cb6c86f9a0f544e8d9b)
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
      data[index-0] = d;
    }
    //[[[end]]] (checksum: 9d402e9f650ebc0f2f5eb923ecab8c0a)
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
      return data[index-0];
    }
    //[[[end]]] (checksum: bebe841769a6348fc48a3a8dd54f6f27)
    return 0;
  }

#if !defined(__AAL_USER__) && !defined(USE_SOFTWARE)
  /*[[[cog
       cog.outl("inline friend void sc_trace(sc_trace_file* tf, const %s& d, const std::string& name) {" % ty)
    ]]]*/
  inline friend void sc_trace(sc_trace_file* tf, const BlkOut& d, const std::string& name) {
  //[[[end]]] (checksum: 289b068911745e450ad3b67c5836eb1e)
  }
#endif

  /*[[[cog
       cog.outl("inline friend std::ostream& operator<<(std::ostream& os, const %s& d) {" % ty)
       cog.outl("  os << \"<%s>\";" % ty)
    ]]]*/
  inline friend std::ostream& operator<<(std::ostream& os, const BlkOut& d) {
    os << "<BlkOut>";
  //[[[end]]] (checksum: b6eb8e74c8939936cbc729761348ea0c)
    return os;
  }

  /*[[[cog
       cog.outl("inline bool operator==(const %s& rhs) const {" % ty)
    ]]]*/
  inline bool operator==(const BlkOut& rhs) const {
  //[[[end]]] (checksum: 666d389ca66c66e31d47c56d23770b89)
    bool result = true;
    for( unsigned int i=0; i<numberOfFields(); ++i) {
      result = result && (getField(i) == rhs.getField(i));
    }
    return result;
  }

};

#endif
