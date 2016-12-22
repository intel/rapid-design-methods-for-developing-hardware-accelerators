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
#include "BWCacheLine.h"
#include "BWResult.h"
#include "BWResultLine.h"
#include "BWPattern.h"
#include "BWState.h"
//[[[end]]] (checksum: d18b31adffc6344855be2ed990ae7fd9)

typedef unsigned long long AddrType;
typedef unsigned int OffsetType;

struct Config {
private:
  /*[[[cog
       for p in dut.inps + dut.outs:
         cog.outl("AddrType a%s : 64;" % p.nm.capitalize())
    ]]]*/
  AddrType aPat : 64;
  AddrType aCl : 64;
  AddrType aPre : 64;
  AddrType aRes : 64;
  //[[[end]]] (checksum: 92450611a0f62a3ce3eb481326b21084)  
  /*[[[cog
       for field in dut.extra_config_fields:
         if type(field) is BitReducedField: 
            cog.outl("%s %s : %d;" % (field.ty.ty,field.ty.nm,field.ty.bitwidth))
         elif type(field) is ArrayField:
            cog.outl("%s %s[%d];" % (field.ty.ty,field.ty.nm,field.ty.count))
         else:
            cog.outl("%s %s;" % (field.ty,field.nm))
    ]]]*/
  unsigned long long nPat : 64;
  unsigned long long max_recirculating : 64;
  unsigned long long precomp_len : 64;
  unsigned int end_pos : 32;
  unsigned int u0 : 32;
  unsigned int u1 : 32;
  unsigned int u2 : 32;
  unsigned int u3 : 32;
  //[[[end]]] (checksum: d1238e5f7885565d65e1e8506180b409)  

public:
  /*[[[cog
       for p in dut.inps + dut.outs:
         cog.outl("AddrType get_a%s() const {" % p.nm.capitalize())
         cog.outl("  return a%s & 0x0000ffffffffffc0ULL;" % p.nm.capitalize())
         cog.outl("}")
    ]]]*/
  AddrType get_aPat() const {
    return aPat & 0x0000ffffffffffc0ULL;
  }
  AddrType get_aCl() const {
    return aCl & 0x0000ffffffffffc0ULL;
  }
  AddrType get_aPre() const {
    return aPre & 0x0000ffffffffffc0ULL;
  }
  AddrType get_aRes() const {
    return aRes & 0x0000ffffffffffc0ULL;
  }
  //[[[end]]] (checksum: 6d15df4a30224e2bac52261e8c664026)  
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
  unsigned long long get_nPat() const {
    return nPat;
  }
  unsigned long long get_max_recirculating() const {
    return max_recirculating;
  }
  unsigned long long get_precomp_len() const {
    return precomp_len;
  }
  unsigned int get_end_pos() const {
    return end_pos;
  }
  unsigned int get_u0() const {
    return u0;
  }
  unsigned int get_u1() const {
    return u1;
  }
  unsigned int get_u2() const {
    return u2;
  }
  unsigned int get_u3() const {
    return u3;
  }
  //[[[end]]] (checksum: dd5310d5d0c6c777eb7663f163c717dd)  

  void set_aVD( const AddrType& val) {
  }
  /*[[[cog
       for p in dut.inps + dut.outs:
         cog.outl("void set_a%s( const AddrType& val) {" % p.nm.capitalize())
         cog.outl("  assert( !(val & ~0x0000ffffffffffc0ULL));")
         cog.outl("  a%s = val;" % p.nm.capitalize())
         cog.outl("}")
    ]]]*/
  void set_aPat( const AddrType& val) {
    assert( !(val & ~0x0000ffffffffffc0ULL));
    aPat = val;
  }
  void set_aCl( const AddrType& val) {
    assert( !(val & ~0x0000ffffffffffc0ULL));
    aCl = val;
  }
  void set_aPre( const AddrType& val) {
    assert( !(val & ~0x0000ffffffffffc0ULL));
    aPre = val;
  }
  void set_aRes( const AddrType& val) {
    assert( !(val & ~0x0000ffffffffffc0ULL));
    aRes = val;
  }
  //[[[end]]] (checksum: 13865caa183dda584a4dde5e3dc14251)  
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
  void set_nPat( const unsigned long long& val) {
    nPat = val;
  }
  void set_max_recirculating( const unsigned long long& val) {
    max_recirculating = val;
  }
  void set_precomp_len( const unsigned long long& val) {
    precomp_len = val;
  }
  void set_end_pos( const unsigned int& val) {
    end_pos = val;
  }
  void set_u0( const unsigned int& val) {
    u0 = val;
  }
  void set_u1( const unsigned int& val) {
    u1 = val;
  }
  void set_u2( const unsigned int& val) {
    u2 = val;
  }
  void set_u3( const unsigned int& val) {
    u3 = val;
  }
  //[[[end]]] (checksum: 14380b438ddf5851fbc5436f00d9b17e)  

  /*[[[cog
       for p in dut.inps + dut.outs:
         cog.outl("AddrType get%sAddr( size_t idx) const {" % p.nm.capitalize())
         cog.outl("  return 0x0000ffffffffffffULL & (get_a%s() + (%s::getBitCnt()/8)*idx);" % (p.nm.capitalize(),p.ty))
         cog.outl("}")
    ]]]*/
  AddrType getPatAddr( size_t idx) const {
    return 0x0000ffffffffffffULL & (get_aPat() + (BWPattern::getBitCnt()/8)*idx);
  }
  AddrType getClAddr( size_t idx) const {
    return 0x0000ffffffffffffULL & (get_aCl() + (BWCacheLine::getBitCnt()/8)*idx);
  }
  AddrType getPreAddr( size_t idx) const {
    return 0x0000ffffffffffffULL & (get_aPre() + (BWResult::getBitCnt()/8)*idx);
  }
  AddrType getResAddr( size_t idx) const {
    return 0x0000ffffffffffffULL & (get_aRes() + (BWResultLine::getBitCnt()/8)*idx);
  }
  //[[[end]]] (checksum: 6890fe3f0baa8c74c3d58242bb02dd9e)  

  /*[[[cog
       for p in dut.inps + dut.outs:
         cog.outl("%s* get%sPtr() const {" % (p.ty,p.nm.capitalize()))
         cog.outl("  return reinterpret_cast<%s*>( get_a%s());" % (p.ty,p.nm.capitalize()))
         cog.outl("}")
    ]]]*/
  BWPattern* getPatPtr() const {
    return reinterpret_cast<BWPattern*>( get_aPat());
  }
  BWCacheLine* getClPtr() const {
    return reinterpret_cast<BWCacheLine*>( get_aCl());
  }
  BWResult* getPrePtr() const {
    return reinterpret_cast<BWResult*>( get_aPre());
  }
  BWResultLine* getResPtr() const {
    return reinterpret_cast<BWResultLine*>( get_aRes());
  }
  //[[[end]]] (checksum: e8fda727faaed5bf344bbebce6cf5efb)  

  /*[[[cog
       for p in dut.inps + dut.outs:
         cog.outl("size_t addr2Idx%s( AddrType addr) const {" % p.nm.capitalize())
         cog.outl("  return (addr - get_a%s()) / ((%s::getBitCnt()/8));" % (p.nm.capitalize(),p.ty))  
         cog.outl("}")
    ]]]*/
  size_t addr2IdxPat( AddrType addr) const {
    return (addr - get_aPat()) / ((BWPattern::getBitCnt()/8));
  }
  size_t addr2IdxCl( AddrType addr) const {
    return (addr - get_aCl()) / ((BWCacheLine::getBitCnt()/8));
  }
  size_t addr2IdxPre( AddrType addr) const {
    return (addr - get_aPre()) / ((BWResult::getBitCnt()/8));
  }
  size_t addr2IdxRes( AddrType addr) const {
    return (addr - get_aRes()) / ((BWResultLine::getBitCnt()/8));
  }
  //[[[end]]] (checksum: 9622dad27bf6626e9d7a6e416220fc01)  

  unsigned int n() const {
    return u3;
  }

  unsigned int m() const {
    //    return (n()+127)/128;
    if ( 0x7f & n())
      return (n()>>7) + 1;
    else
      return (n()>>7);
  }

  unsigned int first( int r, int f) const {
    if        ( r == 0 && f == 0) {
      return 1;
    } else if ( r == 0 && f == 1) {
      return u0;
    } else if ( r == 1 && f == 0) {
      return u0;
    } else if ( r == 1 && f == 1) {
      return u1;
    } else if ( r == 2 && f == 0) {
      return u1;
    } else if ( r == 2 && f == 1) {
      return u2;
    } else if ( r == 3 && f == 0) {
      return u2;
    } else if ( r == 3 && f == 1) {
      return u3;
    } else {
      assert(0);
      return 0;
    }
  }

  Config() {
    /*[[[cog
         for p in dut.inps + dut.outs:
           cog.outl("a%s = 0;" % p.nm.capitalize())
      ]]]*/
    aPat = 0;
    aCl = 0;
    aPre = 0;
    aRes = 0;
    //[[[end]]] (checksum: 5521489d3448ba90ac14c4fa830da447)  
    /*[[[cog
         for field in dut.extra_config_fields:
           cog.outl("%s = 0;" % field.nm)
      ]]]*/
    nPat = 0;
    max_recirculating = 0;
    precomp_len = 0;
    end_pos = 0;
    u0 = 0;
    u1 = 0;
    u2 = 0;
    u3 = 0;
    //[[[end]]] (checksum: e3cbf009796bcb7ac6d52b61047c01d1)  
  }
  void copy(Config &from) {
    /*[[[cog
         for p in dut.inps + dut.outs:
           cog.outl("a%s = from.a%s;" % (p.nm.capitalize(),p.nm.capitalize()))
      ]]]*/
    aPat = from.aPat;
    aCl = from.aCl;
    aPre = from.aPre;
    aRes = from.aRes;
    //[[[end]]] (checksum: b5121a94dbc5678e7609905a7496cb0c)  
    /*[[[cog
         for field in dut.extra_config_fields:
           cog.outl("%s = from.%s;" % (field.nm,field.nm))
      ]]]*/
    nPat = from.nPat;
    max_recirculating = from.max_recirculating;
    precomp_len = from.precomp_len;
    end_pos = from.end_pos;
    u0 = from.u0;
    u1 = from.u1;
    u2 = from.u2;
    u3 = from.u3;
    //[[[end]]] (checksum: 743ba5c23761cbca76a5da82bdbde4e1)  
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
    os << "aPat: " << d.aPat << std::endl;
    os << "aCl: " << d.aCl << std::endl;
    os << "aPre: " << d.aPre << std::endl;
    os << "aRes: " << d.aRes << std::endl;
    //[[[end]]] (checksum: 0c5a87d6cf71bd71caf52a06029bd4ac)  
    /*[[[cog
         for field in dut.extra_config_fields:
           cog.outl("os << \"%s: \" << d.%s << std::endl;" % (field.nm,field.nm))
      ]]]*/
    os << "nPat: " << d.nPat << std::endl;
    os << "max_recirculating: " << d.max_recirculating << std::endl;
    os << "precomp_len: " << d.precomp_len << std::endl;
    os << "end_pos: " << d.end_pos << std::endl;
    os << "u0: " << d.u0 << std::endl;
    os << "u1: " << d.u1 << std::endl;
    os << "u2: " << d.u2 << std::endl;
    os << "u3: " << d.u3 << std::endl;
    //[[[end]]] (checksum: 6827fb6d65f34327426d7edabf9e4d4a)  
    return os;
  }

  inline bool operator==(const Config& rhs) const {
    bool result = true;
    /*[[[cog
         for p in dut.inps + dut.outs:
           cog.outl("result = result && (a%s == rhs.a%s);" % (p.nm.capitalize(),p.nm.capitalize()))
      ]]]*/
    result = result && (aPat == rhs.aPat);
    result = result && (aCl == rhs.aCl);
    result = result && (aPre == rhs.aPre);
    result = result && (aRes == rhs.aRes);
    //[[[end]]] (checksum: ff973b0c4b8002447e428611f4409b90)  
    /*[[[cog
         for field in dut.extra_config_fields:
           cog.outl("result = result && (%s == rhs.%s);" % (field.nm,field.nm))
      ]]]*/
    result = result && (nPat == rhs.nPat);
    result = result && (max_recirculating == rhs.max_recirculating);
    result = result && (precomp_len == rhs.precomp_len);
    result = result && (end_pos == rhs.end_pos);
    result = result && (u0 == rhs.u0);
    result = result && (u1 == rhs.u1);
    result = result && (u2 == rhs.u2);
    result = result && (u3 == rhs.u3);
    //[[[end]]] (checksum: 019b9efccca84fe3b353c475858f46c2)  
    return result;
  }
};

#endif
