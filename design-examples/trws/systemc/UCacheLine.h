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
//ty=UCacheLine
//[[[end]]] (checksum: 1956297a7b98ae3e2ac940557b704b54)
/*[[[cog
     cog.outl("#ifndef %s_H_" % ty)
     cog.outl("#define %s_H_" % ty)
  ]]]*/
#ifndef UCacheLine_H_
#define UCacheLine_H_
//[[[end]]] (checksum: bb63dfe14d6dd0584bad8f74b518fed1)

#ifndef __SYNTHESIS__
#include <cstddef>
#include <cassert>
#endif

/*[[[cog
     cog.outl("class %s {" % ty)
  ]]]*/
class UCacheLine {
//[[[end]]] (checksum: a622d06c8610584034293ee2b5375187)
public:
  /*[[[cog
       for field in ut.fields:
         cog.outl( field.declaration)
    ]]]*/
  unsigned short words[32];
  //[[[end]]] (checksum: 5526c9e849581abd7a4e3e1ed0570bb7)

  /*[[[cog
       if len(ut.fields) == 1 and type(ut.fields[0]) is ArrayField:
         cog.outl( "enum { ArrayLength = %d };" % ut.fields[0].count)
         cog.outl( "typedef %s ElementType;" % ut.fields[0].ty.ty)
    ]]]*/
  enum { ArrayLength = 32 };
  typedef unsigned short ElementType;
  //[[[end]]] (checksum: a8e681eaeb3575ad14d772bf6e0b38fd)

  /*[[[cog
       cog.outl("enum { BitCnt = %d };" % ut.bitwidth)
    ]]]*/
  enum { BitCnt = 512 };
  //[[[end]]] (checksum: 37159abeda6c5f75899fb314e1ed078c)

  static size_t getBitCnt() {
    /*[[[cog
         cog.outl("assert(sizeof(%s) == (size_t) BitCnt/8);" % ty)
      ]]]*/
    assert(sizeof(UCacheLine) == (size_t) BitCnt/8);
    //[[[end]]] (checksum: 6ad6286f30afc0a3789b0386fe7c18bc)
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
  inline friend void sc_trace(sc_trace_file* tf, const UCacheLine& d, const std::string& name) {
  //[[[end]]] (checksum: bbe27f7d653bbdb021b6206341cc3b5f)
  }
#endif

  /*[[[cog
       cog.outl("inline friend std::ostream& operator<<(std::ostream& os, const %s& d) {" % ty)
       cog.outl("  os << \"<%s>\";" % ty)
    ]]]*/
  inline friend std::ostream& operator<<(std::ostream& os, const UCacheLine& d) {
    os << "<UCacheLine>";
  //[[[end]]] (checksum: 7b333653bf0fae1a01b5820cc3b420db)
    return os;
  }

  /*[[[cog
       cog.outl("inline bool operator==(const %s& rhs) const {" % ty)
    ]]]*/
  inline bool operator==(const UCacheLine& rhs) const {
  //[[[end]]] (checksum: d5d2a59a4ab8d2c22b9b46b728197bcd)
    bool result = true;
    for( unsigned int i=0; i<numberOfFields(); ++i) {
      result = result && (getField(i) == rhs.getField(i));
    }
    return result;
  }

};

#endif
