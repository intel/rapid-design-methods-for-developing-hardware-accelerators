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
//[[[end]]] (checksum: 7fb3c950fa8d9b407f933e8d4ee64449)

typedef unsigned long long AddrType;

struct Config {
private:
  /*[[[cog
       for p in dut.inps + dut.outs:
         cog.outl("AddrType a%s : 64;" % p.nm.capitalize())
    ]]]*/
  AddrType aIna : 64;
  AddrType aInb : 64;
  AddrType aOut : 64;
  //[[[end]]] (checksum: ade5dcad8997bed02dd679949c2bcef9)  
  /*[[[cog
       for field in dut.extra_config_fields:
         if type(field) is BitReducedField: 
            cog.outl("%s %s : %d;" % (field.ty.ty,field.ty.nm,field.ty.bitwidth))
         elif type(field) is ArrayField:
            cog.outl("%s %s[%d];" % (field.ty.ty,field.ty.nm,field.ty.count))
         else:
            cog.outl("%s %s;" % (field.ty,field.nm))
    ]]]*/
  unsigned int n : 32;
  //[[[end]]] (checksum: 0b58635e41a595741da9b16d02c1a0cb)  

public:
  /*[[[cog
       for p in dut.inps + dut.outs:
         cog.outl("AddrType get_a%s() const {" % p.nm.capitalize())
         cog.outl("  return a%s & 0x0000ffffffffffc0ULL;" % p.nm.capitalize())
         cog.outl("}")
    ]]]*/
  AddrType get_aIna() const {
    return aIna & 0x0000ffffffffffc0ULL;
  }
  AddrType get_aInb() const {
    return aInb & 0x0000ffffffffffc0ULL;
  }
  AddrType get_aOut() const {
    return aOut & 0x0000ffffffffffc0ULL;
  }
  //[[[end]]] (checksum: f1b7accba8fe1e829d97f4557b4af18d)  
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
  unsigned int get_n() const {
    return n;
  }
  //[[[end]]] (checksum: d16e8e564c82661c2b6262fb3a5b5c36)  

  void set_aVD( const AddrType& val) {
  }
  /*[[[cog
       for p in dut.inps + dut.outs:
         cog.outl("void set_a%s( const AddrType& val) {" % p.nm.capitalize())
         cog.outl("  assert( !(val & ~0x0000ffffffffffc0ULL));")
         cog.outl("  a%s = val;" % p.nm.capitalize())
         cog.outl("}")
    ]]]*/
  void set_aIna( const AddrType& val) {
    assert( !(val & ~0x0000ffffffffffc0ULL));
    aIna = val;
  }
  void set_aInb( const AddrType& val) {
    assert( !(val & ~0x0000ffffffffffc0ULL));
    aInb = val;
  }
  void set_aOut( const AddrType& val) {
    assert( !(val & ~0x0000ffffffffffc0ULL));
    aOut = val;
  }
  //[[[end]]] (checksum: eb5b9bbc93154679170879230ca95684)  
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
  void set_n( const unsigned int& val) {
    n = val;
  }
  //[[[end]]] (checksum: cc9d445927831b68c690034a38eea257)  

  /*[[[cog
       for p in dut.inps + dut.outs:
         cog.outl("AddrType get%sAddr( size_t idx) const {" % p.nm.capitalize())
         cog.outl("  return 0x0000ffffffffffffULL & (get_a%s() + (%s::getBitCnt()/8)*idx);" % (p.nm.capitalize(),p.ty))
         cog.outl("}")
    ]]]*/
  AddrType getInaAddr( size_t idx) const {
    return 0x0000ffffffffffffULL & (get_aIna() + (Blk::getBitCnt()/8)*idx);
  }
  AddrType getInbAddr( size_t idx) const {
    return 0x0000ffffffffffffULL & (get_aInb() + (Blk::getBitCnt()/8)*idx);
  }
  AddrType getOutAddr( size_t idx) const {
    return 0x0000ffffffffffffULL & (get_aOut() + (Blk::getBitCnt()/8)*idx);
  }
  //[[[end]]] (checksum: afb41a907f1987ea222f9b847de3c8fa)  

  /*[[[cog
       for p in dut.inps + dut.outs:
         cog.outl("%s* get%sPtr() const {" % (p.ty,p.nm.capitalize()))
         cog.outl("  return reinterpret_cast<%s*>( get_a%s());" % (p.ty,p.nm.capitalize()))
         cog.outl("}")
    ]]]*/
  Blk* getInaPtr() const {
    return reinterpret_cast<Blk*>( get_aIna());
  }
  Blk* getInbPtr() const {
    return reinterpret_cast<Blk*>( get_aInb());
  }
  Blk* getOutPtr() const {
    return reinterpret_cast<Blk*>( get_aOut());
  }
  //[[[end]]] (checksum: 6902f31f49a62b2e8828594c5045942d)  

  /*[[[cog
       for p in dut.inps + dut.outs:
         cog.outl("size_t addr2Idx%s( AddrType addr) const {" % p.nm.capitalize())
         cog.outl("  return (addr - get_a%s()) / ((%s::getBitCnt()/8));" % (p.nm.capitalize(),p.ty))  
         cog.outl("}")
    ]]]*/
  size_t addr2IdxIna( AddrType addr) const {
    return (addr - get_aIna()) / ((Blk::getBitCnt()/8));
  }
  size_t addr2IdxInb( AddrType addr) const {
    return (addr - get_aInb()) / ((Blk::getBitCnt()/8));
  }
  size_t addr2IdxOut( AddrType addr) const {
    return (addr - get_aOut()) / ((Blk::getBitCnt()/8));
  }
  //[[[end]]] (checksum: 14b4b8a443663e21226616409d45bea1)  

  Config() {
    /*[[[cog
         for p in dut.inps + dut.outs:
           cog.outl("a%s = 0;" % p.nm.capitalize())
      ]]]*/
    aIna = 0;
    aInb = 0;
    aOut = 0;
    //[[[end]]] (checksum: f9d22ede4e7462f3cfe611b401aa7eb5)  
    /*[[[cog
         for field in dut.extra_config_fields:
           cog.outl("%s = 0;" % field.nm)
      ]]]*/
    n = 0;
    //[[[end]]] (checksum: 6ca13aae88a7c5322e1d189d6facbdca)  
  }
  void copy(Config &from) {
    /*[[[cog
         for p in dut.inps + dut.outs:
           cog.outl("a%s = from.a%s;" % (p.nm.capitalize(),p.nm.capitalize()))
      ]]]*/
    aIna = from.aIna;
    aInb = from.aInb;
    aOut = from.aOut;
    //[[[end]]] (checksum: 1860d6dcd35ffa8325c0e3b0fd0d4675)  
    /*[[[cog
         for field in dut.extra_config_fields:
           cog.outl("%s = from.%s;" % (field.nm,field.nm))
      ]]]*/
    n = from.n;
    //[[[end]]] (checksum: 58624d720d96f90bbbae6998b53c2762)  
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
    os << "aIna: " << d.aIna << std::endl;
    os << "aInb: " << d.aInb << std::endl;
    os << "aOut: " << d.aOut << std::endl;
    //[[[end]]] (checksum: 450aacecf726c1e0ec2213b39fd642af)  
    /*[[[cog
         for field in dut.extra_config_fields:
           cog.outl("os << \"%s: \" << d.%s << std::endl;" % (field.nm,field.nm))
      ]]]*/
    os << "n: " << d.n << std::endl;
    //[[[end]]] (checksum: 4841c82dfbe7eb9abb9b4b4de51e4ff6)  
    return os;
  }

  inline bool operator==(const Config& rhs) const {
    bool result = true;
    /*[[[cog
         for p in dut.inps + dut.outs:
           cog.outl("result = result && (a%s == rhs.a%s);" % (p.nm.capitalize(),p.nm.capitalize()))
      ]]]*/
    result = result && (aIna == rhs.aIna);
    result = result && (aInb == rhs.aInb);
    result = result && (aOut == rhs.aOut);
    //[[[end]]] (checksum: f82aa26c11e6bf6b4dfc649129ae3af2)  
    /*[[[cog
         for field in dut.extra_config_fields:
           cog.outl("result = result && (%s == rhs.%s);" % (field.nm,field.nm))
      ]]]*/
    result = result && (n == rhs.n);
    //[[[end]]] (checksum: 1ded3fa3d31128bf645461746ea61950)  
    return result;
  }
};

#endif
