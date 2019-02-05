// See LICENSE for license details.
/*[[[cog
     import cog
     from cog_acctempl import *
     from dut_params import *
     print(dut.ports_without_an_address, dut.ports_with_an_address)
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
#include "CacheLine.h"
#include "UCacheLine.h"
#include "WrappedShort.h"
#include "Pair.h"
//[[[end]]] (checksum: 33e1959ee013d128677d2c4f1a454be2)

typedef unsigned long long AddrType;

struct Config {
private:
  /*[[[cog
       for p in dut.ports_with_an_address:
         cog.outl("AddrType a%s : 64;" % p.nm.capitalize())
    ]]]*/
  AddrType aGi : 64;
  AddrType aWi : 64;
  AddrType aMi : 64;
  AddrType aInp : 64;
  AddrType aMo : 64;
  //[[[end]]] (checksum: 2aae88cd7529e4178b34b68446436beb)  
  /*[[[cog
       for field in dut.extra_config_fields:
         if type(field) is BitReducedField: 
            cog.outl("%s %s : %d;" % (field.ty.ty,field.ty.nm,field.bitwidth))
         elif type(field) is ArrayField:
            cog.outl("%s %s[%d];" % (field.ty.ty,field.ty.nm,field.ty.count))
         else:
            cog.outl("%s %s;" % (field.ty,field.nm))
    ]]]*/
  unsigned int nCLperRow;
  unsigned int nSlices;
  //[[[end]]] (checksum: 7ed74ce8d222b7a7ca57b088a203184e)  

public:
  /*[[[cog
       for p in dut.ports_with_an_address:
         cog.outl("AddrType get_a%s() const {" % p.nm.capitalize())
         cog.outl("  return a%s & 0x0000ffffffffffc0ULL;" % p.nm.capitalize())
         cog.outl("}")
    ]]]*/
  AddrType get_aGi() const {
    return aGi & 0x0000ffffffffffc0ULL;
  }
  AddrType get_aWi() const {
    return aWi & 0x0000ffffffffffc0ULL;
  }
  AddrType get_aMi() const {
    return aMi & 0x0000ffffffffffc0ULL;
  }
  AddrType get_aInp() const {
    return aInp & 0x0000ffffffffffc0ULL;
  }
  AddrType get_aMo() const {
    return aMo & 0x0000ffffffffffc0ULL;
  }
  //[[[end]]] (checksum: 7cd9caa53eae90af9e1e2f4a9092c9f0)  
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
  unsigned int get_nCLperRow() const {
    return nCLperRow;
  }
  unsigned int get_nSlices() const {
    return nSlices;
  }
  //[[[end]]] (checksum: dcf0eef0a4609e399bfb557e9b20e26c)  

  void set_aVD( const AddrType& val) {
  }
  /*[[[cog
       for p in dut.ports_with_an_address:
         cog.outl("void set_a%s( const AddrType& val) {" % p.nm.capitalize())
         cog.outl("  assert( !(val & ~0x0000ffffffffffc0ULL));")
         cog.outl("  a%s = val;" % p.nm.capitalize())
         cog.outl("}")
    ]]]*/
  void set_aGi( const AddrType& val) {
    assert( !(val & ~0x0000ffffffffffc0ULL));
    aGi = val;
  }
  void set_aWi( const AddrType& val) {
    assert( !(val & ~0x0000ffffffffffc0ULL));
    aWi = val;
  }
  void set_aMi( const AddrType& val) {
    assert( !(val & ~0x0000ffffffffffc0ULL));
    aMi = val;
  }
  void set_aInp( const AddrType& val) {
    assert( !(val & ~0x0000ffffffffffc0ULL));
    aInp = val;
  }
  void set_aMo( const AddrType& val) {
    assert( !(val & ~0x0000ffffffffffc0ULL));
    aMo = val;
  }
  //[[[end]]] (checksum: aca4761463d20d80ff7bc78abc3b72b2)  
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
  void set_nCLperRow( const unsigned int& val) {
    nCLperRow = val;
  }
  void set_nSlices( const unsigned int& val) {
    nSlices = val;
  }
  //[[[end]]] (checksum: 673586c1cb5e645e5ac7e60315d2bc61)  

  /*[[[cog
       for p in dut.ports_with_an_address:
         cog.outl("AddrType get%sAddr( size_t idx) const {" % p.nm.capitalize())
         cog.outl("  return 0x0000ffffffffffffULL & (get_a%s() + (%s::getBitCnt()/8)*idx);" % (p.nm.capitalize(),p.ty))
         cog.outl("}")
    ]]]*/
  AddrType getGiAddr( size_t idx) const {
    return 0x0000ffffffffffffULL & (get_aGi() + (UCacheLine::getBitCnt()/8)*idx);
  }
  AddrType getWiAddr( size_t idx) const {
    return 0x0000ffffffffffffULL & (get_aWi() + (CacheLine::getBitCnt()/8)*idx);
  }
  AddrType getMiAddr( size_t idx) const {
    return 0x0000ffffffffffffULL & (get_aMi() + (UCacheLine::getBitCnt()/8)*idx);
  }
  AddrType getInpAddr( size_t idx) const {
    return 0x0000ffffffffffffULL & (get_aInp() + (Pair::getBitCnt()/8)*idx);
  }
  AddrType getMoAddr( size_t idx) const {
    return 0x0000ffffffffffffULL & (get_aMo() + (UCacheLine::getBitCnt()/8)*idx);
  }
  //[[[end]]] (checksum: ca24233050ae07c470e5a8306d481359)  

  /*[[[cog
       for p in dut.ports_with_an_address:
         cog.outl("%s* get%sPtr() const {" % (p.ty,p.nm.capitalize()))
         cog.outl("  return reinterpret_cast<%s*>( get_a%s());" % (p.ty,p.nm.capitalize()))
         cog.outl("}")
    ]]]*/
  UCacheLine* getGiPtr() const {
    return reinterpret_cast<UCacheLine*>( get_aGi());
  }
  CacheLine* getWiPtr() const {
    return reinterpret_cast<CacheLine*>( get_aWi());
  }
  UCacheLine* getMiPtr() const {
    return reinterpret_cast<UCacheLine*>( get_aMi());
  }
  Pair* getInpPtr() const {
    return reinterpret_cast<Pair*>( get_aInp());
  }
  UCacheLine* getMoPtr() const {
    return reinterpret_cast<UCacheLine*>( get_aMo());
  }
  //[[[end]]] (checksum: 6e8dc67630ada687f31334a6fa287f61)  

  /*[[[cog
       for p in dut.ports_with_an_address:
         cog.outl("size_t addr2Idx%s( AddrType addr) const {" % p.nm.capitalize())
         cog.outl("  return (addr - get_a%s()) / ((%s::getBitCnt()/8));" % (p.nm.capitalize(),p.ty))  
         cog.outl("}")
    ]]]*/
  size_t addr2IdxGi( AddrType addr) const {
    return (addr - get_aGi()) / ((UCacheLine::getBitCnt()/8));
  }
  size_t addr2IdxWi( AddrType addr) const {
    return (addr - get_aWi()) / ((CacheLine::getBitCnt()/8));
  }
  size_t addr2IdxMi( AddrType addr) const {
    return (addr - get_aMi()) / ((UCacheLine::getBitCnt()/8));
  }
  size_t addr2IdxInp( AddrType addr) const {
    return (addr - get_aInp()) / ((Pair::getBitCnt()/8));
  }
  size_t addr2IdxMo( AddrType addr) const {
    return (addr - get_aMo()) / ((UCacheLine::getBitCnt()/8));
  }
  //[[[end]]] (checksum: 033889147385aba3a00eaa7a72ed353a)  

  Config() {
    /*[[[cog
         for p in dut.ports_with_an_address:
           cog.outl("a%s = 0;" % p.nm.capitalize())
      ]]]*/
    aGi = 0;
    aWi = 0;
    aMi = 0;
    aInp = 0;
    aMo = 0;
    //[[[end]]] (checksum: e8e5bfd1143346279d29371caa72916f)  
    /*[[[cog
         for field in dut.extra_config_fields:
           cog.outl("%s = 0;" % field.nm)
      ]]]*/
    nCLperRow = 0;
    nSlices = 0;
    //[[[end]]] (checksum: 1de0c32d6f3ae2e2615a583f6bb6c970)  
  }
  void copy(Config &from) {
    /*[[[cog
         for p in dut.ports_with_an_address:
           cog.outl("a%s = from.a%s;" % (p.nm.capitalize(),p.nm.capitalize()))
      ]]]*/
    aGi = from.aGi;
    aWi = from.aWi;
    aMi = from.aMi;
    aInp = from.aInp;
    aMo = from.aMo;
    //[[[end]]] (checksum: 20d2f1e30ae4b5144f3cdf4f91ee617e)  
    /*[[[cog
         for field in dut.extra_config_fields:
           cog.outl("%s = from.%s;" % (field.nm,field.nm))
      ]]]*/
    nCLperRow = from.nCLperRow;
    nSlices = from.nSlices;
    //[[[end]]] (checksum: 8b6a1ce187fa16c2e79d5b708e56942e)  
  }

#if !defined(__AAL_USER__) && !defined(USE_SOFTWARE)
  inline friend void sc_trace(sc_trace_file* tf, const Config& d,
      const std::string& name) {
  }
#endif

  inline friend std::ostream& operator<<(std::ostream& os, const Config& d) {
    /*[[[cog
         for p in dut.ports_with_an_address:
           cog.outl("os << \"a%s: \" << d.a%s << std::endl;" % (p.nm.capitalize(),p.nm.capitalize()))
      ]]]*/
    os << "aGi: " << d.aGi << std::endl;
    os << "aWi: " << d.aWi << std::endl;
    os << "aMi: " << d.aMi << std::endl;
    os << "aInp: " << d.aInp << std::endl;
    os << "aMo: " << d.aMo << std::endl;
    //[[[end]]] (checksum: 9ab7c756a7f40bc121cb2cefb4ce6571)  
    /*[[[cog
         for field in dut.extra_config_fields:
           cog.outl("os << \"%s: \" << d.%s << std::endl;" % (field.nm,field.nm))
      ]]]*/
    os << "nCLperRow: " << d.nCLperRow << std::endl;
    os << "nSlices: " << d.nSlices << std::endl;
    //[[[end]]] (checksum: e996e0bf04c5096c60d4bc351b6ddec3)  
    return os;
  }

  inline bool operator==(const Config& rhs) const {
    bool result = true;
    /*[[[cog
         for p in dut.ports_with_an_address:
           cog.outl("result = result && (a%s == rhs.a%s);" % (p.nm.capitalize(),p.nm.capitalize()))
      ]]]*/
    result = result && (aGi == rhs.aGi);
    result = result && (aWi == rhs.aWi);
    result = result && (aMi == rhs.aMi);
    result = result && (aInp == rhs.aInp);
    result = result && (aMo == rhs.aMo);
    //[[[end]]] (checksum: e4ff7892169aeb6859bf17fbe9ddafec)  
    /*[[[cog
         for field in dut.extra_config_fields:
           cog.outl("result = result && (%s == rhs.%s);" % (field.nm,field.nm))
      ]]]*/
    result = result && (nCLperRow == rhs.nCLperRow);
    result = result && (nSlices == rhs.nSlices);
    //[[[end]]] (checksum: c7bfddd6b63c8a6677651d9216f694f7)  
    return result;
  }
};

#endif
