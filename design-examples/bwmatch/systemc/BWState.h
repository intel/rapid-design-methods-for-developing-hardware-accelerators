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
//ty=BWState
//[[[end]]] (checksum: 414db907bb92606d64dcf0671ddd7a53)
/*[[[cog
     cog.outl("#ifndef %s_H_" % ty)
     cog.outl("#define %s_H_" % ty)
  ]]]*/
#ifndef BWState_H_
#define BWState_H_
//[[[end]]] (checksum: 4c0ebb2296a64ba2c2967acd3cbef9a3)

#ifndef __SYNTHESIS__
#include <cstddef>
#include <cassert>
#endif

/*[[[cog
     cog.outl("class %s {" % ty)
  ]]]*/
class BWState {
//[[[end]]] (checksum: e721034d3e0e8f0f0c52429d5ec5f844)
public:
  /*[[[cog
       for field in ut.fields:
         cog.outl( field.declaration)
    ]]]*/
  unsigned int idx;
  BWResult res;
  BWPattern pat;
  unsigned int state : 2;
  //[[[end]]] (checksum: 132459cf58ba0d13b6c90ef0b5760731)

  /*[[[cog
       cog.outl("enum { BitCnt = %d };" % ut.bitwidth)
    ]]]*/
  enum { BitCnt = 162 };
  //[[[end]]] (checksum: b075c1b28813f8e75d1a438a54a68d41)

  static size_t getBitCnt() {
    /*[[[cog
         cog.outl("assert(sizeof(%s) == (size_t) BitCnt/8);" % ty)
      ]]]*/
    assert(sizeof(BWState) == (size_t) BitCnt/8);
    //[[[end]]] (checksum: c534ae5a4d2aa03018da297f9490e3db)
    assert( 0 == (size_t) BitCnt%8);
    return BitCnt;
  }
  static size_t numberOfFields() {
    /*[[[cog
         cog.outl("return %d;" % ut.numberOfFields)
      ]]]*/
    return 6;
    //[[[end]]] (checksum: a0728dffb01272905b62f92a658370f2)
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
    if ( 1 <= index && index < 3) {
      return BWResult::fieldWidth( index-1);
    }
    if ( 3 <= index && index < 5) {
      return BWPattern::fieldWidth( index-3);
    }
    if ( 5 <= index && index < 6) {
      return 2;
    }
    //[[[end]]] (checksum: 7c66569154097022dd19572a0bd06c4f)
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
      idx = d;
    }
    if ( 1 <= index && index < 3) {
      res.putField( index-1, d);
    }
    if ( 3 <= index && index < 5) {
      pat.putField( index-3, d);
    }
    if ( 5 <= index && index < 6) {
      state = d;
    }
    //[[[end]]] (checksum: c31ed7737f26ccaba463d1fbb795b742)
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
      return idx;
    }
    if ( 1 <= index && index < 3) {
      return res.getField( index-1);
    }
    if ( 3 <= index && index < 5) {
      return pat.getField( index-3);
    }
    if ( 5 <= index && index < 6) {
      return state;
    }
    //[[[end]]] (checksum: 2e782d963735ae9c27b168f32d071638)
    return 0;
  }

#if !defined(__AAL_USER__) && !defined(USE_SOFTWARE)
  /*[[[cog
       cog.outl("inline friend void sc_trace(sc_trace_file* tf, const %s& d, const std::string& name) {" % ty)
    ]]]*/
  inline friend void sc_trace(sc_trace_file* tf, const BWState& d, const std::string& name) {
  //[[[end]]] (checksum: ef8bc01719ae250693b977dcb4cf5b32)
  }
#endif

  /*[[[cog
       cog.outl("inline friend std::ostream& operator<<(std::ostream& os, const %s& d) {" % ty)
       cog.outl("  os << \"<%s>\";" % ty)
    ]]]*/
  inline friend std::ostream& operator<<(std::ostream& os, const BWState& d) {
    os << "<BWState>";
  //[[[end]]] (checksum: 1a84de6fc557811d2a2e0b1c568ab82e)
    return os;
  }

  /*[[[cog
       cog.outl("inline bool operator==(const %s& rhs) const {" % ty)
    ]]]*/
  inline bool operator==(const BWState& rhs) const {
  //[[[end]]] (checksum: 692f93717c62ec533e59f0d9b039ac18)
    bool result = true;
    for( unsigned int i=0; i<numberOfFields(); ++i) {
      result = result && (getField(i) == rhs.getField(i));
    }
    return result;
  }

};

#endif
