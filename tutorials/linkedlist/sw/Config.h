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
     for k in usertypes.keys():
       cog.outl("#include \"%s.h\"" % k)
  ]]]*/
#include "Node.h"
#include "CycleExistInfo.h"
//[[[end]]] (checksum: 5ff3dfa5d7bc4335c0d427c52fed7d84)

typedef unsigned long long AddrType;
typedef unsigned int OffsetType;

struct Config {
private:
  AddrType aVD : 64;

  /*[[[cog
       for p in inps + outs:
         cog.outl("OffsetType o%s : 32;" % p.nm.capitalize())
    ]]]*/
  OffsetType oInp : 32;
  OffsetType oOut : 32;
  //[[[end]]] (checksum: 792bfb6b7d9896ad07035732d131c6cd)  
  /*[[[cog
       for field in extra_config_fields:
         if type(field) is BitReducedField: 
            cog.outl("%s %s : %d;" % (field.ty.ty,field.ty.nm,field.ty.bitwidth))
         elif type(field) is ArrayField:
            cog.outl("%s %s[%d];" % (field.ty.ty,field.ty.nm,field.ty.count))
         else:
            cog.outl("%s %s;" % (field.ty,field.nm))
    ]]]*/
  //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)  

public:
  AddrType get_aVD() const {
    return aVD;
  }
  /*[[[cog
       for p in inps + outs:
         cog.outl("OffsetType get_o%s() const {" % p.nm.capitalize())
         cog.outl("  return o%s;" % p.nm.capitalize())
         cog.outl("}")
    ]]]*/
  OffsetType get_oInp() const {
    return oInp;
  }
  OffsetType get_oOut() const {
    return oOut;
  }
  //[[[end]]] (checksum: 4fd284706fdc73ca046c5cda2cca9b17)  
  /*[[[cog
       for field in extra_config_fields:
         if type(field) is ArrayField: continue
         if type(field) is BitReducedField:
           cog.outl("%s get_%s() const {" % (field.ty.ty,field.nm))
         else:
           cog.outl("%s get_%s() const {" % (field.ty,field.nm))
         cog.outl("  return %s;" % field.nm)
         cog.outl("}")
    ]]]*/
  //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)  

  void set_aVD( const AddrType& val) {
    aVD = val;
  }
  /*[[[cog
       for p in inps + outs:
         cog.outl("void set_o%s( const OffsetType& val) {" % p.nm.capitalize())
         cog.outl("  o%s = val;" % p.nm.capitalize())
         cog.outl("}")
    ]]]*/
  void set_oInp( const OffsetType& val) {
    oInp = val;
  }
  void set_oOut( const OffsetType& val) {
    oOut = val;
  }
  //[[[end]]] (checksum: b4d4c244269907bf5b41d947163aa72e)  
  /*[[[cog
       for field in extra_config_fields:
         if type(field) is ArrayField: continue
         if type(field) is BitReducedField:
           local_ty = field.ty.ty
         else:
           local_ty = field.ty
         cog.outl("void set_%s( const %s& val) {" % (field.nm,local_ty))
         cog.outl("  %s = val;" % field.nm)
         cog.outl("}")
    ]]]*/
  //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)  

  /*[[[cog
       for p in inps + outs:
         cog.outl("AddrType get%sAddr( unsigned int idx) const {" % p.nm.capitalize())
         cog.outl("  return aVD + get_o%s() + (%s::getBitCnt()/8)*idx;" % (p.nm.capitalize(),p.ty))
         cog.outl("}")
    ]]]*/
  AddrType getInpAddr( unsigned int idx) const {
    return aVD + get_oInp() + (Node::getBitCnt()/8)*idx;
  }
  AddrType getOutAddr( unsigned int idx) const {
    return aVD + get_oOut() + (CycleExistInfo::getBitCnt()/8)*idx;
  }
  //[[[end]]] (checksum: c36c52674ce31593c894d3f092e9ac26)  

  /*[[[cog
       for p in inps + outs:
         cog.outl("unsigned int addr2Idx%s( AddrType addr) const {" % p.nm.capitalize())
         cog.outl("  return (addr - aVD - get_o%s()) / ((%s::getBitCnt()/8));" % (p.nm.capitalize(),p.ty))  
         cog.outl("}")
    ]]]*/
  unsigned int addr2IdxInp( AddrType addr) const {
    return (addr - aVD - get_oInp()) / ((Node::getBitCnt()/8));
  }
  unsigned int addr2IdxOut( AddrType addr) const {
    return (addr - aVD - get_oOut()) / ((CycleExistInfo::getBitCnt()/8));
  }
  //[[[end]]] (checksum: f46ab7637b074aa29277ebcecb73c315)  

  Config() {
    aVD = 0;
    /*[[[cog
         for p in inps + outs:
           cog.outl("o%s = 0;" % p.nm.capitalize())
      ]]]*/
    oInp = 0;
    oOut = 0;
    //[[[end]]] (checksum: 94c8c5b7f708a4ddf199c2b4b0106037)  
    /*[[[cog
         for field in extra_config_fields:
           cog.outl("%s = 0;" % field.nm)
      ]]]*/
    //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)  
  }
  void copy(Config &from) {
    aVD = from.aVD;
    /*[[[cog
         for p in inps + outs:
           cog.outl("o%s = from.o%s;" % (p.nm.capitalize(),p.nm.capitalize()))
      ]]]*/
    oInp = from.oInp;
    oOut = from.oOut;
    //[[[end]]] (checksum: 65c412cbc2a8844938f9371b77772654)  
    /*[[[cog
         for field in extra_config_fields:
           cog.outl("%s = from.%s;" % (field.nm,field.nm))
      ]]]*/
    //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)  
  }

#if !defined(__AAL_USER__) && !defined(USE_SOFTWARE)
  inline friend void sc_trace(sc_trace_file* tf, const Config& d,
      const std::string& name) {
  }
#endif

  inline friend std::ostream& operator<<(std::ostream& os, const Config& d) {
    os << "aVD: " << d.aVD << std::endl;
    /*[[[cog
         for p in inps + outs:
           cog.outl("os << \"o%s: \" << d.o%s << std::endl;" % (p.nm.capitalize(),p.nm.capitalize()))
      ]]]*/
    os << "oInp: " << d.oInp << std::endl;
    os << "oOut: " << d.oOut << std::endl;
    //[[[end]]] (checksum: 94d3d9f24f2590438cb659df47a1da3d)  
    /*[[[cog
         for field in extra_config_fields:
           cog.outl("os << \"%s: \" << d.%s << std::endl;" % (field.nm,field.nm))
      ]]]*/
    //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)  
    return os;
  }

  inline bool operator==(const Config& rhs) const {
    bool result = true;
    result = result && (aVD == rhs.aVD);
    /*[[[cog
         for p in inps + outs:
           cog.outl("result = result && (o%s == rhs.o%s);" % (p.nm.capitalize(),p.nm.capitalize()))
      ]]]*/
    result = result && (oInp == rhs.oInp);
    result = result && (oOut == rhs.oOut);
    //[[[end]]] (checksum: 246b66dc592d0a163817706653ea675c)  
    /*[[[cog
         for field in extra_config_fields:
           cog.outl("result = result && (%s == rhs.%s);" % (field.nm,field.nm))
      ]]]*/
    //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)  
    return result;
  }
};

#endif
