// See LICENSE for license details.
/*[[[cog
     import cog
     from cog_acctempl import *
     from dut_params import *
     ports = [ x for x in dut.inps + dut.outs if x.nm != "lst"]
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
#include "HeadPtr.h"
#include "Node.h"
//[[[end]]] (checksum: 416e5717e6ec4b7d9174881df865f5ae)

typedef unsigned long long AddrType;

struct Config {
private:
  /*[[[cog
       for p in ports:
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
  unsigned long long m;
  //[[[end]]] (checksum: f53e9408de20463606ffb5212d88192a)  

public:
  /*[[[cog
       for p in ports:
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
  unsigned long long get_m() const {
    return m;
  }
  //[[[end]]] (checksum: d2cb95481e21148ddb6a2bbeca7e8256)  

  void set_aVD( const AddrType& val) {
  }
  /*[[[cog
       for p in ports:
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
  void set_m( const unsigned long long& val) {
    m = val;
  }
  //[[[end]]] (checksum: 6b1d9a33b696797144116a3696aaec5f)  

  /*[[[cog
       for p in ports:
         cog.outl("AddrType get%sAddr( size_t idx) const {" % p.nm.capitalize())
         cog.outl("  return 0x0000ffffffffffffULL & (get_a%s() + (%s::getBitCnt()/8)*idx);" % (p.nm.capitalize(),p.ty))
         cog.outl("}")
    ]]]*/
  AddrType getInpAddr( size_t idx) const {
    return 0x0000ffffffffffffULL & (get_aInp() + (HeadPtr::getBitCnt()/8)*idx);
  }
  AddrType getOutAddr( size_t idx) const {
    return 0x0000ffffffffffffULL & (get_aOut() + (HeadPtr::getBitCnt()/8)*idx);
  }
  //[[[end]]] (checksum: 05748ea885b63fbccf00fa6030f1a5c2)  

  /*[[[cog
       for p in ports:
         cog.outl("%s* get%sPtr() const {" % (p.ty,p.nm.capitalize()))
         cog.outl("  return reinterpret_cast<%s*>( get_a%s());" % (p.ty,p.nm.capitalize()))
         cog.outl("}")
    ]]]*/
  HeadPtr* getInpPtr() const {
    return reinterpret_cast<HeadPtr*>( get_aInp());
  }
  HeadPtr* getOutPtr() const {
    return reinterpret_cast<HeadPtr*>( get_aOut());
  }
  //[[[end]]] (checksum: a058ea4140a6765ec7c83c590645252c)  

  /*[[[cog
       for p in ports:
         cog.outl("size_t addr2Idx%s( AddrType addr) const {" % p.nm.capitalize())
         cog.outl("  return (addr - get_a%s()) / ((%s::getBitCnt()/8));" % (p.nm.capitalize(),p.ty))  
         cog.outl("}")
    ]]]*/
  size_t addr2IdxInp( AddrType addr) const {
    return (addr - get_aInp()) / ((HeadPtr::getBitCnt()/8));
  }
  size_t addr2IdxOut( AddrType addr) const {
    return (addr - get_aOut()) / ((HeadPtr::getBitCnt()/8));
  }
  //[[[end]]] (checksum: 773b1fb935dd17ee939ac0561ed645ff)  

  Config() {
    /*[[[cog
         for p in ports:
           cog.outl("a%s = 0;" % p.nm.capitalize())
      ]]]*/
    aInp = 0;
    aOut = 0;
    //[[[end]]] (checksum: 631579fca416802271757551ee21cf23)  
    /*[[[cog
         for field in dut.extra_config_fields:
           cog.outl("%s = 0;" % field.nm)
      ]]]*/
    m = 0;
    //[[[end]]] (checksum: 7a41e90171196f5d36decd8f32b2197d)  
  }
  void copy(Config &from) {
    /*[[[cog
         for p in ports:
           cog.outl("a%s = from.a%s;" % (p.nm.capitalize(),p.nm.capitalize()))
      ]]]*/
    aInp = from.aInp;
    aOut = from.aOut;
    //[[[end]]] (checksum: baf9bbf79dc699771e9388ea8c1c471f)  
    /*[[[cog
         for field in dut.extra_config_fields:
           cog.outl("%s = from.%s;" % (field.nm,field.nm))
      ]]]*/
    m = from.m;
    //[[[end]]] (checksum: 3580feabf5983e563b53efd6e979bb23)  
  }

#if !defined(__AAL_USER__) && !defined(USE_SOFTWARE)
  inline friend void sc_trace(sc_trace_file* tf, const Config& d,
      const std::string& name) {
  }
#endif

  inline friend std::ostream& operator<<(std::ostream& os, const Config& d) {
    /*[[[cog
         for p in ports:
           cog.outl("os << \"a%s: \" << d.a%s << std::endl;" % (p.nm.capitalize(),p.nm.capitalize()))
      ]]]*/
    os << "aInp: " << d.aInp << std::endl;
    os << "aOut: " << d.aOut << std::endl;
    //[[[end]]] (checksum: 55d463caad0d9c86717df63edb65ea94)  
    /*[[[cog
         for field in dut.extra_config_fields:
           cog.outl("os << \"%s: \" << d.%s << std::endl;" % (field.nm,field.nm))
      ]]]*/
    os << "m: " << d.m << std::endl;
    //[[[end]]] (checksum: 4964bf914630efb52d0de4004adb1c06)  
    return os;
  }

  inline bool operator==(const Config& rhs) const {
    bool result = true;
    /*[[[cog
         for p in ports:
           cog.outl("result = result && (a%s == rhs.a%s);" % (p.nm.capitalize(),p.nm.capitalize()))
      ]]]*/
    result = result && (aInp == rhs.aInp);
    result = result && (aOut == rhs.aOut);
    //[[[end]]] (checksum: ef8aba184f75d9b48b8e1fab32f6a09b)  
    /*[[[cog
         for field in dut.extra_config_fields:
           cog.outl("result = result && (%s == rhs.%s);" % (field.nm,field.nm))
      ]]]*/
    result = result && (m == rhs.m);
    //[[[end]]] (checksum: 0ec3d898db1623550a1afae3a00cc01d)  
    return result;
  }
};

#endif
