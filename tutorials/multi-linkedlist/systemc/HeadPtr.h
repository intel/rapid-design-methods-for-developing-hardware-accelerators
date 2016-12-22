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
//ty=HeadPtr
//[[[end]]] (checksum: e84f1f64680458c37696e0ddc0e0ac1f)
/*[[[cog
     cog.outl("#ifndef %s_H_" % ty)
     cog.outl("#define %s_H_" % ty)
  ]]]*/
#ifndef HeadPtr_H_
#define HeadPtr_H_
//[[[end]]] (checksum: 1616d4f5ad8c9cae22097c2af6fb49f7)

#ifndef __SYNTHESIS__
#include <cstddef>
#include <cassert>
#endif

class Node;

/*[[[cog
     cog.outl("class %s {" % ty)
  ]]]*/
class HeadPtr {
//[[[end]]] (checksum: 2a12d540710fb84903aa2bc1f473096b)
public:
  /*[[[cog
       for field in ut.fields:
         cog.outl( field.declaration)
    ]]]*/
  unsigned long long head;
  //[[[end]]] (checksum: 511a18547689ac7fdd1d52227f83ec8b)

  Node *get_head() const {
    return reinterpret_cast<Node *>( head);
  }

  void set_head( Node *node) {
    head = reinterpret_cast<unsigned long long>( node);
  }

  bool get_found() const {
    return head & 0x1ULL;
  }

  void set_found( bool val) {
    if ( val) {
      head = head |  0x1ULL;
    } else {
      head = head & ~0x1ULL;
    }
  }

  /*[[[cog
       cog.outl("enum { BitCnt = %d };" % ut.bitwidth)
    ]]]*/
  enum { BitCnt = 64 };
  //[[[end]]] (checksum: 5b4922b42a8ba86e82bf3f7c903a4156)

  static size_t getBitCnt() {
    /*[[[cog
         cog.outl("assert(sizeof(%s) == (size_t) BitCnt/8);" % ty)
      ]]]*/
    assert(sizeof(HeadPtr) == (size_t) BitCnt/8);
    //[[[end]]] (checksum: fec3b95b9d2b17d22b20911d28e29f99)
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
      return 64;
    }
    //[[[end]]] (checksum: ea856f278e2695b0a7981ed82902fa6f)
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
      head = d;
    }
    //[[[end]]] (checksum: c062f1a27ff0b1296caa40020df580de)
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
      return head;
    }
    //[[[end]]] (checksum: ff7e1a07aa20fcf1311e7c6fa5c3b47d)
    return 0;
  }

#if !defined(__AAL_USER__) && !defined(USE_SOFTWARE)
  /*[[[cog
       cog.outl("inline friend void sc_trace(sc_trace_file* tf, const %s& d, const std::string& name) {" % ty)
    ]]]*/
  inline friend void sc_trace(sc_trace_file* tf, const HeadPtr& d, const std::string& name) {
  //[[[end]]] (checksum: 04e40899fd8c484b8162dfe1b3f226b5)
  }
#endif

  /*[[[cog
       cog.outl("inline friend std::ostream& operator<<(std::ostream& os, const %s& d) {" % ty)
       cog.outl("  os << \"<%s>\";" % ty)
    ]]]*/
  inline friend std::ostream& operator<<(std::ostream& os, const HeadPtr& d) {
    os << "<HeadPtr>";
  //[[[end]]] (checksum: 7374ccf81709a8124cf0e897ff367822)
    return os;
  }

  /*[[[cog
       cog.outl("inline bool operator==(const %s& rhs) const {" % ty)
    ]]]*/
  inline bool operator==(const HeadPtr& rhs) const {
  //[[[end]]] (checksum: 6b561e024481fb1380e6f27169555e84)
    bool result = true;
    for( unsigned int i=0; i<numberOfFields(); ++i) {
      result = result && (getField(i) == rhs.getField(i));
    }
    return result;
  }

};

#endif
