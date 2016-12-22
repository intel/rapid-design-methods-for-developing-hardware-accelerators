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
//ty=BWCacheLine
//[[[end]]] (checksum: dc39df375ff657304d2784d551cc2114)
/*[[[cog
     cog.outl("#ifndef %s_H_" % ty)
     cog.outl("#define %s_H_" % ty)
  ]]]*/
#ifndef BWCacheLine_H_
#define BWCacheLine_H_
//[[[end]]] (checksum: a2384e90d69f68367f56b99dcc6e1918)

#ifndef __SYNTHESIS__
#include <cstddef>
#include <cassert>
#endif

/*[[[cog
     cog.outl("class %s {" % ty)
  ]]]*/
class BWCacheLine {
//[[[end]]] (checksum: 618293feaa5b4aef5b5cf8a640383d38)
public:
  /*[[[cog
       for field in ut.fields:
         cog.outl( field.declaration)
    ]]]*/
  unsigned long long bwa[4];
  unsigned int ranks[4];
  unsigned long long pad[2];
  //[[[end]]] (checksum: c6e61aed846ec732a62be58e97000832)

  /*[[[cog
       cog.outl("enum { BitCnt = %d };" % ut.bitwidth)
    ]]]*/
  enum { BitCnt = 512 };
  //[[[end]]] (checksum: 37159abeda6c5f75899fb314e1ed078c)

  static size_t getBitCnt() {
    /*[[[cog
         cog.outl("assert(sizeof(%s) == (size_t) BitCnt/8);" % ty)
      ]]]*/
    assert(sizeof(BWCacheLine) == (size_t) BitCnt/8);
    //[[[end]]] (checksum: ab6c9cea1c67e7eb3fe8d5b708c84265)
    assert( 0 == (size_t) BitCnt%8);
    return BitCnt;
  }
  static size_t numberOfFields() {
    /*[[[cog
         cog.outl("return %d;" % ut.numberOfFields)
      ]]]*/
    return 10;
    //[[[end]]] (checksum: 3ee45546b6e8e9b4d00565234569180a)
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
    if ( 0 <= index && index < 4) {
      return 64;
    }
    if ( 4 <= index && index < 8) {
      return 32;
    }
    if ( 8 <= index && index < 10) {
      return 64;
    }
    //[[[end]]] (checksum: a73e6742b06af2dbc2cc37b9e53b5e97)
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
    if ( 0 <= index && index < 4) {
      bwa[index-0] = d;
    }
    if ( 4 <= index && index < 8) {
      ranks[index-4] = d;
    }
    if ( 8 <= index && index < 10) {
      pad[index-8] = d;
    }
    //[[[end]]] (checksum: bdbb38c47fe4e195969ddcc05e7d044f)
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
    if ( 0 <= index && index < 4) {
      return bwa[index-0];
    }
    if ( 4 <= index && index < 8) {
      return ranks[index-4];
    }
    if ( 8 <= index && index < 10) {
      return pad[index-8];
    }
    //[[[end]]] (checksum: e169bb5e3b280773fbe17fd31f55da6c)
    return 0;
  }

#if !defined(__AAL_USER__) && !defined(USE_SOFTWARE)
  /*[[[cog
       cog.outl("inline friend void sc_trace(sc_trace_file* tf, const %s& d, const std::string& name) {" % ty)
    ]]]*/
  inline friend void sc_trace(sc_trace_file* tf, const BWCacheLine& d, const std::string& name) {
  //[[[end]]] (checksum: b2eba8aa6913f85eb31bec9a906e84ae)
  }
#endif

  /*[[[cog
       cog.outl("inline friend std::ostream& operator<<(std::ostream& os, const %s& d) {" % ty)
       cog.outl("  os << \"<%s>\";" % ty)
    ]]]*/
  inline friend std::ostream& operator<<(std::ostream& os, const BWCacheLine& d) {
    os << "<BWCacheLine>";
  //[[[end]]] (checksum: e54df0b30bf354a868727d3c41043243)
    return os;
  }

  /*[[[cog
       cog.outl("inline bool operator==(const %s& rhs) const {" % ty)
    ]]]*/
  inline bool operator==(const BWCacheLine& rhs) const {
  //[[[end]]] (checksum: 597452dbb5b6b25d5b02e9cd5187f89f)
    bool result = true;
    for( unsigned int i=0; i<numberOfFields(); ++i) {
      result = result && (getField(i) == rhs.getField(i));
    }
    return result;
  }

};

#endif
