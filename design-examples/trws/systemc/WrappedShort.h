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
//ty=WrappedShort
//[[[end]]] (checksum: ccd6ed07b779cbbcb0c4bbc6698bfad2)
/*[[[cog
     cog.outl("#ifndef %s_H_" % ty)
     cog.outl("#define %s_H_" % ty)
  ]]]*/
#ifndef WrappedShort_H_
#define WrappedShort_H_
//[[[end]]] (checksum: 49e4303404b68adf8ed89dc5b8901834)

#ifndef __SYNTHESIS__
#include <cstddef>
#include <cassert>
#endif

/*[[[cog
     cog.outl("class %s {" % ty)
  ]]]*/
class WrappedShort {
//[[[end]]] (checksum: 46c72a3915168e8db4c108b2daeb8ccd)
public:
  /*[[[cog
       for field in ut.fields:
         cog.outl( field.declaration)
    ]]]*/
  short data;
  //[[[end]]] (checksum: 835d6ef0e9c5ac258ae29e5343f02db3)

  /*[[[cog
       if len(ut.fields) == 1 and type(ut.fields[0]) is ArrayField:
         cog.outl( "enum { ArrayLength = %d };" % ut.fields[0].count)
         cog.outl( "typedef %s ElementType;" % ut.fields[0].ty.ty)
    ]]]*/
  //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)

  /*[[[cog
       cog.outl("enum { BitCnt = %d };" % ut.bitwidth)
    ]]]*/
  enum { BitCnt = 16 };
  //[[[end]]] (checksum: c6a2971a11f55b2906dac6750d4e47b0)

  static size_t getBitCnt() {
    /*[[[cog
         cog.outl("assert(sizeof(%s) == (size_t) BitCnt/8);" % ty)
      ]]]*/
    assert(sizeof(WrappedShort) == (size_t) BitCnt/8);
    //[[[end]]] (checksum: 4a916216b2cbee769a9d4293c1578598)
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
      return 16;
    }
    //[[[end]]] (checksum: a0f02c28e3e95004af9aebda90c84f89)
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
      data = d;
    }
    //[[[end]]] (checksum: 3ee2dab495712810642a9c0e1b690090)
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
      return data;
    }
    //[[[end]]] (checksum: 82c3d398b0e8b536d00254950eb5e699)
    return 0;
  }

#if !defined(__AAL_USER__) && !defined(USE_SOFTWARE)
  /*[[[cog
       cog.outl("inline friend void sc_trace(sc_trace_file* tf, const %s& d, const std::string& name) {" % ty)
    ]]]*/
  inline friend void sc_trace(sc_trace_file* tf, const WrappedShort& d, const std::string& name) {
  //[[[end]]] (checksum: bfaed0e45037f35d3023b0798938371a)
  }
#endif

  /*[[[cog
       cog.outl("inline friend std::ostream& operator<<(std::ostream& os, const %s& d) {" % ty)
       cog.outl("  os << \"<%s>\";" % ty)
    ]]]*/
  inline friend std::ostream& operator<<(std::ostream& os, const WrappedShort& d) {
    os << "<WrappedShort>";
  //[[[end]]] (checksum: ceec65f35f8b586417e50d1a358b9de9)
    return os;
  }

  /*[[[cog
       cog.outl("inline bool operator==(const %s& rhs) const {" % ty)
    ]]]*/
  inline bool operator==(const WrappedShort& rhs) const {
  //[[[end]]] (checksum: 25d61aa39f6d3d65fd34ce60d862f747)
    bool result = true;
    for( unsigned int i=0; i<numberOfFields(); ++i) {
      result = result && (getField(i) == rhs.getField(i));
    }
    return result;
  }

};

#endif
