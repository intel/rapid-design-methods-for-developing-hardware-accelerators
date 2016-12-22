// See LICENSE for license details.
/*[[[cog
     import cog
     from cog_acctempl import *
     from dut_params import *
  ]]]*/
//[[[end]]]

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
//[[[end]]]

typedef unsigned long long AddrType;
typedef unsigned int OffsetType;

struct Config {
private:
  AddrType aVD : 64;

  /*[[[cog
       for p in inps + outs:
         cog.outl("OffsetType o%s : 32;" % p.nm.capitalize())
    ]]]*/
  //[[[end]]]  
  /*[[[cog
       for field in extra_config_fields:
         if type(field) is BitReducedField: 
            cog.outl("%s %s : %d;" % (field.ty.ty,field.ty.nm,field.ty.bitwidth))
         elif type(field) is ArrayField:
            cog.outl("%s %s[%d];" % (field.ty.ty,field.ty.nm,field.ty.count))
         else:
            cog.outl("%s %s;" % (field.ty,field.nm))
    ]]]*/
  //[[[end]]]  

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
  //[[[end]]]  
  /*[[[cog
       for p in inps + outs:
         cog.outl("AddrType get_a%s() const {" % p.nm.capitalize())
         cog.outl("  return aVD + o%s;" % p.nm.capitalize())
         cog.outl("}")
    ]]]*/
  //[[[end]]]  
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
  //[[[end]]]  

  void set_aVD( const AddrType& val) {
    aVD = val;
  }
  /*[[[cog
       for p in inps + outs:
         cog.outl("void set_o%s( const OffsetType& val) {" % p.nm.capitalize())
         cog.outl("  o%s = val;" % p.nm.capitalize())
         cog.outl("}")
    ]]]*/
  //[[[end]]]  
  /*[[[cog
       for p in inps + outs:
         cog.outl("void set_a%s( const AddrType& val) {" % p.nm.capitalize())
         cog.outl("  o%s = val - aVD;" % p.nm.capitalize())
         cog.outl("}")
    ]]]*/
  //[[[end]]]  
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
  //[[[end]]]  

  /*[[[cog
       for p in inps + outs:
         cog.outl("AddrType get%sAddr( unsigned int idx) const {" % p.nm.capitalize())
         cog.outl("  return get_a%s() + (%s::getBitCnt()/8)*idx;" % (p.nm.capitalize(),p.ty))
         cog.outl("}")
    ]]]*/
  //[[[end]]]  

  /*[[[cog
       for p in inps + outs:
         cog.outl("unsigned int addr2Idx%s( AddrType addr) const {" % p.nm.capitalize())
         cog.outl("  return (addr - get_a%s()) / ((%s::getBitCnt()/8));" % (p.nm.capitalize(),p.ty))  
         cog.outl("}")
    ]]]*/
  //[[[end]]]  

  Config() {
    aVD = 0;
    /*[[[cog
         for p in inps + outs:
           cog.outl("o%s = 0;" % p.nm.capitalize())
      ]]]*/
    //[[[end]]]  
    /*[[[cog
         for field in extra_config_fields:
           cog.outl("%s = 0;" % field.nm)
      ]]]*/
    //[[[end]]]  
  }
  void copy(Config &from) {
    aVD = from.aVD;
    /*[[[cog
         for p in inps + outs:
           cog.outl("o%s = from.o%s;" % (p.nm.capitalize(),p.nm.capitalize()))
      ]]]*/
    //[[[end]]]  
    /*[[[cog
         for field in extra_config_fields:
           cog.outl("%s = from.%s;" % (field.nm,field.nm))
      ]]]*/
    //[[[end]]]  
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
    //[[[end]]]  
    /*[[[cog
         for field in extra_config_fields:
           cog.outl("os << \"%s: \" << d.%s << std::endl;" % (field.nm,field.nm))
      ]]]*/
    //[[[end]]]  
    return os;
  }

  inline bool operator==(const Config& rhs) const {
    bool result = true;
    result = result && (aVD == rhs.aVD);
    /*[[[cog
         for p in inps + outs:
           cog.outl("result = result && (o%s == rhs.o%s);" % (p.nm.capitalize(),p.nm.capitalize()))
      ]]]*/
    //[[[end]]]  
    /*[[[cog
         for field in extra_config_fields:
           cog.outl("result = result && (%s == rhs.%s);" % (field.nm,field.nm))
      ]]]*/
    //[[[end]]]  
    return result;
  }
};

#endif
