// See LICENSE for license details.
/*[[[cog
     import cog
     from cog_acctempl import *
     from dut_params import *
  ]]]*/
//[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)

#ifndef __CONFIG_H__
#define __CONFIG_H__

#ifndef __SYNTHESIS__
#include <iomanip>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#endif
using std::endl ;
using std::cout ;
using std::string ;

/*[[[cog
     for k in dut.usertypes.keys():
       cog.outl("#include \"%s.h\"" % k)
  ]]]*/
#include "Blk.h"
#include "BlkInp.h"
#include "BlkOut.h"
#include "BlkMid.h"
//[[[end]]] (checksum: 27f96a7aea340393cd00d479ad12a79f)

typedef unsigned long long AddrType;

struct Config {
private:
  /*[[[cog
       for p in dut.inps + dut.outs:
         cog.outl("AddrType a%s : 64;" % p.nm.capitalize())
    ]]]*/
  AddrType aInp : 64;
  AddrType aOut : 64;
  //[[[end]]] (checksum: 1137e5f30f3f0f526629ebbdb5357538)  
  /*[[[cog
       for field in dut.extra_config_fields:
         if type(field) is BitReducedField: 
            cog.outl("%s %s : %d;" % (field.ty.ty,field.ty.nm,field.ty.bitwidth))
         elif type(field) is ArrayField:
            cog.outl("%s %s[%d];" % (field.ty.ty,field.ty.nm,field.ty.count))
         else:
            cog.outl("%s %s;" % (field.ty,field.nm))
    ]]]*/
  unsigned int nInp;
  unsigned int num_of_rows;
  unsigned int row_size_in_blks;
  unsigned int image_size_in_bytes;
  unsigned int num_of_images;
  //[[[end]]] (checksum: 9dff01dcdb163b8e251b787a1b4050b0)  

public:
  /*[[[cog
       for p in dut.inps + dut.outs:
         cog.outl("AddrType get_a%s() const {" % p.nm.capitalize())
         cog.outl("  return a%s & 0x0000ffffffffffc0ULL;" % p.nm.capitalize())
         cog.outl("}")
    ]]]*/
  AddrType get_aInp() const {
    return aInp & 0x0000ffffffffffc0ULL;
  }
  AddrType get_aOut() const {
    return aOut & 0x0000ffffffffffc0ULL;
  }
  //[[[end]]] (checksum: a017975dbccaa1f8269fc429f4e0d911)  
  /*[[[cog
       for field in dut.extra_config_fields:
         if type(field) is ArrayField: continue
         if type(field) is BitReducedField:
           cog.outl("%s get_%s() const {" % (field.ty.ty,field.nm))
         else:
           cog.outl("%s get_%s() const {" % (field.ty,field.nm))
         cog.outl("  return %s;" % field.nm)
         cog.outl("}")
    ]]]*/
  unsigned int get_nInp() const {
    return nInp;
  }
  unsigned int get_num_of_rows() const {
    return num_of_rows;
  }
  unsigned int get_row_size_in_blks() const {
    return row_size_in_blks;
  }
  unsigned int get_image_size_in_bytes() const {
    return image_size_in_bytes;
  }
  unsigned int get_num_of_images() const {
    return num_of_images;
  }
  //[[[end]]] (checksum: 0b9bbcb79cd56b01de95e3c69effcc2a)  

  void set_aVD( const AddrType& val) {
  }
  /*[[[cog
       for p in dut.inps + dut.outs:
         cog.outl("void set_a%s( const AddrType& val) {" % p.nm.capitalize())
         cog.outl("  assert( !(val & ~0x0000ffffffffffc0ULL));")
         cog.outl("  a%s = val;" % p.nm.capitalize())
         cog.outl("}")
    ]]]*/
  void set_aInp( const AddrType& val) {
    assert( !(val & ~0x0000ffffffffffc0ULL));
    aInp = val;
  }
  void set_aOut( const AddrType& val) {
    assert( !(val & ~0x0000ffffffffffc0ULL));
    aOut = val;
  }
  //[[[end]]] (checksum: 986a367196505db0ed3261eb14c3c782)  
  /*[[[cog
       for field in dut.extra_config_fields:
         if type(field) is ArrayField: continue
         if type(field) is BitReducedField:
           local_ty = field.ty.ty
         else:
           local_ty = field.ty
         cog.outl("void set_%s( const %s& val) {" % (field.nm,local_ty))
         cog.outl("  %s = val;" % field.nm)
         cog.outl("}")
    ]]]*/
  void set_nInp( const unsigned int& val) {
    nInp = val;
  }
  void set_num_of_rows( const unsigned int& val) {
    num_of_rows = val;
  }
  void set_row_size_in_blks( const unsigned int& val) {
    row_size_in_blks = val;
  }
  void set_image_size_in_bytes( const unsigned int& val) {
    image_size_in_bytes = val;
  }
  void set_num_of_images( const unsigned int& val) {
    num_of_images = val;
  }
  //[[[end]]] (checksum: a7ab5b1f7746603590947bcd184e8e51)  

  /*[[[cog
       for p in dut.inps + dut.outs:
         cog.outl("AddrType get%sAddr( size_t idx) const {" % p.nm.capitalize())
         cog.outl("  return 0x0000ffffffffffffULL & (get_a%s() + (%s::getBitCnt()/8)*idx);" % (p.nm.capitalize(),p.ty))
         cog.outl("}")
    ]]]*/
  AddrType getInpAddr( size_t idx) const {
    return 0x0000ffffffffffffULL & (get_aInp() + (BlkInp::getBitCnt()/8)*idx);
  }
  AddrType getOutAddr( size_t idx) const {
    return 0x0000ffffffffffffULL & (get_aOut() + (BlkOut::getBitCnt()/8)*idx);
  }
  //[[[end]]] (checksum: 8a5bec805f752d2ebed5c8806e453d4f)  

  /*[[[cog
       for p in dut.inps + dut.outs:
         cog.outl("%s* get%sPtr() const {" % (p.ty,p.nm.capitalize()))
         cog.outl("  return reinterpret_cast<%s*>( get_a%s());" % (p.ty,p.nm.capitalize()))
         cog.outl("}")
    ]]]*/
  BlkInp* getInpPtr() const {
    return reinterpret_cast<BlkInp*>( get_aInp());
  }
  BlkOut* getOutPtr() const {
    return reinterpret_cast<BlkOut*>( get_aOut());
  }
  //[[[end]]] (checksum: 4e5c06a2e8de224ae9485899434ee3a8)  

  /*[[[cog
       for p in dut.inps + dut.outs:
         cog.outl("size_t addr2Idx%s( AddrType addr) const {" % p.nm.capitalize())
         cog.outl("  return (addr - get_a%s()) / ((%s::getBitCnt()/8));" % (p.nm.capitalize(),p.ty))  
         cog.outl("}")
    ]]]*/
  size_t addr2IdxInp( AddrType addr) const {
    return (addr - get_aInp()) / ((BlkInp::getBitCnt()/8));
  }
  size_t addr2IdxOut( AddrType addr) const {
    return (addr - get_aOut()) / ((BlkOut::getBitCnt()/8));
  }
  //[[[end]]] (checksum: c3fe68f1f52b986699ec5d790b029859)  

  Config() {
    /*[[[cog
         for p in dut.inps + dut.outs:
           cog.outl("a%s = 0;" % p.nm.capitalize())
      ]]]*/
    aInp = 0;
    aOut = 0;
    //[[[end]]] (checksum: 631579fca416802271757551ee21cf23)  
    /*[[[cog
         for field in dut.extra_config_fields:
           cog.outl("%s = 0;" % field.nm)
      ]]]*/
    nInp = 0;
    num_of_rows = 0;
    row_size_in_blks = 0;
    image_size_in_bytes = 0;
    num_of_images = 0;
    //[[[end]]] (checksum: dd9932ce2592d109f20c452380a0eeb4)  
  }
  void copy(Config &from) {
    /*[[[cog
         for p in dut.inps + dut.outs:
           cog.outl("a%s = from.a%s;" % (p.nm.capitalize(),p.nm.capitalize()))
      ]]]*/
    aInp = from.aInp;
    aOut = from.aOut;
    //[[[end]]] (checksum: baf9bbf79dc699771e9388ea8c1c471f)  
    /*[[[cog
         for field in dut.extra_config_fields:
           cog.outl("%s = from.%s;" % (field.nm,field.nm))
      ]]]*/
    nInp = from.nInp;
    num_of_rows = from.num_of_rows;
    row_size_in_blks = from.row_size_in_blks;
    image_size_in_bytes = from.image_size_in_bytes;
    num_of_images = from.num_of_images;
    //[[[end]]] (checksum: 4eadab85ef1f76c9a5fb96da71c2eae9)  
  }

#if !defined(__AAL_USER__) && !defined(USE_SOFTWARE)
  inline friend void sc_trace(sc_trace_file* tf, const Config& d,
      const std::string& name) {
  }
#endif

  inline friend std::ostream& operator<<(std::ostream& os, const Config& d) {
    /*[[[cog
         for p in dut.inps + dut.outs:
           cog.outl("os << \"a%s: \" << d.a%s << std::endl;" % (p.nm.capitalize(),p.nm.capitalize()))
      ]]]*/
    os << "aInp: " << d.aInp << std::endl;
    os << "aOut: " << d.aOut << std::endl;
    //[[[end]]] (checksum: 55d463caad0d9c86717df63edb65ea94)  
    /*[[[cog
         for field in dut.extra_config_fields:
           cog.outl("os << \"%s: \" << d.%s << std::endl;" % (field.nm,field.nm))
      ]]]*/
    os << "nInp: " << d.nInp << std::endl;
    os << "num_of_rows: " << d.num_of_rows << std::endl;
    os << "row_size_in_blks: " << d.row_size_in_blks << std::endl;
    os << "image_size_in_bytes: " << d.image_size_in_bytes << std::endl;
    os << "num_of_images: " << d.num_of_images << std::endl;
    //[[[end]]] (checksum: c458223cc1b8b45c23e07fa3ac8ebf95)  
    return os;
  }

  inline bool operator==(const Config& rhs) const {
    bool result = true;
    /*[[[cog
         for p in dut.inps + dut.outs:
           cog.outl("result = result && (a%s == rhs.a%s);" % (p.nm.capitalize(),p.nm.capitalize()))
      ]]]*/
    result = result && (aInp == rhs.aInp);
    result = result && (aOut == rhs.aOut);
    //[[[end]]] (checksum: ef8aba184f75d9b48b8e1fab32f6a09b)  
    /*[[[cog
         for field in dut.extra_config_fields:
           cog.outl("result = result && (%s == rhs.%s);" % (field.nm,field.nm))
      ]]]*/
    result = result && (nInp == rhs.nInp);
    result = result && (num_of_rows == rhs.num_of_rows);
    result = result && (row_size_in_blks == rhs.row_size_in_blks);
    result = result && (image_size_in_bytes == rhs.image_size_in_bytes);
    result = result && (num_of_images == rhs.num_of_images);
    //[[[end]]] (checksum: c1cb76162c8ae7c9814562d49c7a669f)  
    return result;
  }
};

#endif
