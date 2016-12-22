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
//ty=CycleExistInfo
//[[[end]]] (checksum: 0be682165455ee30b75be6c803729649)
/*[[[cog
     cog.outl("#ifndef %s_H_" % ty)
     cog.outl("#define %s_H_" % ty)
  ]]]*/
#ifndef CycleExistInfo_H_
#define CycleExistInfo_H_
//[[[end]]] (checksum: 0d58aea3ff2aaa2d53048de93e34ee09)

#ifndef __SYNTHESIS__
#include <cstddef>
#include <cassert>
#endif

/*[[[cog
     cog.outl("class %s {" % ty)
  ]]]*/
class CycleExistInfo {
//[[[end]]] (checksum: c2aa59abec0e9feaecc50b7bf83f2148)
public:
  /*[[[cog
       for field in ut.fields:
         cog.outl( field.declaration)
    ]]]*/
  unsigned int info;
  //[[[end]]] (checksum: 5ecf0162ad7b7b935b5d19a75b7f65a9)

  /*[[[cog
       cog.outl("enum { BitCnt = %d };" % ut.bitwidth)
    ]]]*/
  enum { BitCnt = 32 };
  //[[[end]]] (checksum: bb636e1fc432e8e460875ab4d601f55b)

  CycleExistInfo() {}
  CycleExistInfo(bool val) : info(val){}
  
  static size_t getBitCnt() {
    /*[[[cog
         cog.outl("assert(sizeof(%s) == (size_t) BitCnt/8);" % ty)
      ]]]*/
    assert(sizeof(CycleExistInfo) == (size_t) BitCnt/8);
    //[[[end]]] (checksum: 66b3be90ee00c9de4ae51f9d8fa6fcfb)
    assert( 0 == (size_t) BitCnt%8);
    return BitCnt;
  }
  static size_t numberOfFields() {
    /*[[[cog
         cog.outl("return %d;" % ut.numberOfFields)
      ]]]*/
    return 1;
    //[[[end]]] (checksum: cfdb01589c51e51490ce5ba975a468a3)
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
    //[[[end]]] (checksum: 9cbe798004ad46e962214c00424c74e9)
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
      info = d;
    }
    //[[[end]]] (checksum: 9ae64685abca3c8a90a15518a854c251)
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
      return info;
    }
    //[[[end]]] (checksum: eb40dbc8256f1a78e2b41678afa6e892)
    return 0;
  }

#if !defined(__AAL_USER__) && !defined(USE_SOFTWARE)
  /*[[[cog
       cog.outl("inline friend void sc_trace(sc_trace_file* tf, const %s& d, const std::string& name) {" % ty)
    ]]]*/
  inline friend void sc_trace(sc_trace_file* tf, const CycleExistInfo& d, const std::string& name) {
  //[[[end]]] (checksum: fae6707ad5df621340c27b2d79b47097)
  }
#endif

  /*[[[cog
       cog.outl("inline friend std::ostream& operator<<(std::ostream& os, const %s& d) {" % ty)
       cog.outl("  os << \"<%s>\";" % ty)
    ]]]*/
  inline friend std::ostream& operator<<(std::ostream& os, const CycleExistInfo& d) {
    os << "<CycleExistInfo>";
  //[[[end]]] (checksum: 225f3284f8b2b0df4ece9d48d0f43215)
    return os;
  }

  /*[[[cog
       cog.outl("inline bool operator==(const %s& rhs) const {" % ty)
    ]]]*/
  inline bool operator==(const CycleExistInfo& rhs) const {
  //[[[end]]] (checksum: 8c1abef6495376e1e053b5c20c1ff617)
    bool result = true;
    for( unsigned int i=0; i<numberOfFields(); ++i) {
      result = result && (getField(i) == rhs.getField(i));
    }
    return result;
  }

};

#endif
