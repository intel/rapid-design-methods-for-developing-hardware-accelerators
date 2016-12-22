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
//ty=BWResult
//[[[end]]] (checksum: c196307eccf68a363312310c6103bc11)
/*[[[cog
     cog.outl("#ifndef %s_H_" % ty)
     cog.outl("#define %s_H_" % ty)
  ]]]*/
#ifndef BWResult_H_
#define BWResult_H_
//[[[end]]] (checksum: 4ab2e1006a79db626e5a7bc6a2a57a1d)

#ifndef __SYNTHESIS__
#include <cstddef>
#include <cassert>
#endif

/*[[[cog
     cog.outl("class %s {" % ty)
  ]]]*/
class BWResult {
//[[[end]]] (checksum: ae913d4d0deca743075cb6ef82864fd7)
public:
  /*[[[cog
       for field in ut.fields:
         cog.outl( field.declaration)
    ]]]*/
  unsigned int l : 32;
  unsigned int u : 32;
  //[[[end]]] (checksum: 4041fe4830af301f9452680c08641a7d)

  /*[[[cog
       cog.outl("enum { BitCnt = %d };" % ut.bitwidth)
    ]]]*/
  enum { BitCnt = 64 };
  //[[[end]]] (checksum: 5b4922b42a8ba86e82bf3f7c903a4156)

  static size_t getBitCnt() {
    /*[[[cog
         cog.outl("assert(sizeof(%s) == (size_t) BitCnt/8);" % ty)
      ]]]*/
    assert(sizeof(BWResult) == (size_t) BitCnt/8);
    //[[[end]]] (checksum: 18cea4307fc571cbfd173286d38651cf)
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
      return 32;
    }
    if ( 1 <= index && index < 2) {
      return 32;
    }
    //[[[end]]] (checksum: 15cc75f567f835329448304c99138a8d)
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
      l = d;
    }
    if ( 1 <= index && index < 2) {
      u = d;
    }
    //[[[end]]] (checksum: 8d67014b0d2f489f58a9f6c5224766d3)
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
      return l;
    }
    if ( 1 <= index && index < 2) {
      return u;
    }
    //[[[end]]] (checksum: 02f33fded3042fcf08ab123187306842)
    return 0;
  }

#if !defined(__AAL_USER__) && !defined(USE_SOFTWARE)
  /*[[[cog
       cog.outl("inline friend void sc_trace(sc_trace_file* tf, const %s& d, const std::string& name) {" % ty)
    ]]]*/
  inline friend void sc_trace(sc_trace_file* tf, const BWResult& d, const std::string& name) {
  //[[[end]]] (checksum: de31fc9f49a25acd43984a772390d04b)
  }
#endif

  /*[[[cog
       cog.outl("inline friend std::ostream& operator<<(std::ostream& os, const %s& d) {" % ty)
       cog.outl("  os << \"<%s>\";" % ty)
    ]]]*/
  inline friend std::ostream& operator<<(std::ostream& os, const BWResult& d) {
    os << "<BWResult>";
  //[[[end]]] (checksum: 9f49de83571c85e89d26bb135db67715)
    return os;
  }

  /*[[[cog
       cog.outl("inline bool operator==(const %s& rhs) const {" % ty)
    ]]]*/
  inline bool operator==(const BWResult& rhs) const {
  //[[[end]]] (checksum: 319977096e484c7e0b2fed02ecb505aa)
    bool result = true;
    for( unsigned int i=0; i<numberOfFields(); ++i) {
      result = result && (getField(i) == rhs.getField(i));
    }
    return result;
  }

};

#endif
