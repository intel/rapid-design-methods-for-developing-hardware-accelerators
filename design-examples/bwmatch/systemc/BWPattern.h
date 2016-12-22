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
//ty=BWPattern
//[[[end]]] (checksum: 50b0cc7ee880b46ef9c9657c37c0e987)
/*[[[cog
     cog.outl("#ifndef %s_H_" % ty)
     cog.outl("#define %s_H_" % ty)
  ]]]*/
#ifndef BWPattern_H_
#define BWPattern_H_
//[[[end]]] (checksum: 59502b98bbaab52ad12b43343efcb3b0)

#ifndef __SYNTHESIS__
#include <cstddef>
#include <cassert>
#endif

/*[[[cog
     cog.outl("class %s {" % ty)
  ]]]*/
class BWPattern {
//[[[end]]] (checksum: 98762a9bc39c480c4bdb8d271cd75fe0)
public:
  /*[[[cog
       for field in ut.fields:
         cog.outl( field.declaration)
    ]]]*/
  unsigned long long pat : 58;
  unsigned long long length : 6;
  //[[[end]]] (checksum: ae1f825efdeae48a2dbd668c96c19882)

  /*[[[cog
       cog.outl("enum { BitCnt = %d };" % ut.bitwidth)
    ]]]*/
  enum { BitCnt = 64 };
  //[[[end]]] (checksum: 5b4922b42a8ba86e82bf3f7c903a4156)

  static size_t getBitCnt() {
    /*[[[cog
         cog.outl("assert(sizeof(%s) == (size_t) BitCnt/8);" % ty)
      ]]]*/
    assert(sizeof(BWPattern) == (size_t) BitCnt/8);
    //[[[end]]] (checksum: a0962a14d4f54091e4a521ed57fbac77)
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
      return 58;
    }
    if ( 1 <= index && index < 2) {
      return 6;
    }
    //[[[end]]] (checksum: 7fafa25b8aa30ce35b1bfd7def1a7706)
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
      pat = d;
    }
    if ( 1 <= index && index < 2) {
      length = d;
    }
    //[[[end]]] (checksum: 0a083d195c3934729f1791389ab03bff)
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
      return pat;
    }
    if ( 1 <= index && index < 2) {
      return length;
    }
    //[[[end]]] (checksum: 9d7e919a923b15d6c3d71307a77c4f3e)
    return 0;
  }

#if !defined(__AAL_USER__) && !defined(USE_SOFTWARE)
  /*[[[cog
       cog.outl("inline friend void sc_trace(sc_trace_file* tf, const %s& d, const std::string& name) {" % ty)
    ]]]*/
  inline friend void sc_trace(sc_trace_file* tf, const BWPattern& d, const std::string& name) {
  //[[[end]]] (checksum: 058e54d282b15204d7eb0ea354c2bc0a)
  }
#endif

  /*[[[cog
       cog.outl("inline friend std::ostream& operator<<(std::ostream& os, const %s& d) {" % ty)
       cog.outl("  os << \"<%s>\";" % ty)
    ]]]*/
  inline friend std::ostream& operator<<(std::ostream& os, const BWPattern& d) {
    os << "<BWPattern>";
  //[[[end]]] (checksum: 4ecf02b3189373758eab8dc2dc9b1246)
    return os;
  }

  /*[[[cog
       cog.outl("inline bool operator==(const %s& rhs) const {" % ty)
    ]]]*/
  inline bool operator==(const BWPattern& rhs) const {
  //[[[end]]] (checksum: c3670349d8c9e2fd65afe5aba2335773)
    bool result = true;
    for( unsigned int i=0; i<numberOfFields(); ++i) {
      result = result && (getField(i) == rhs.getField(i));
    }
    return result;
  }

};

#endif
