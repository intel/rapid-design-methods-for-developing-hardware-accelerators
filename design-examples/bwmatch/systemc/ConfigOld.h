// See LICENSE for license details.
/*[[[cog
     import cog
     from cog_acctempl import *
     from dut_params import *
  ]]]*/
//[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)

#ifndef __CONFIGOLD_H__
#define __CONFIGOLD_H__

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
#include "BWCacheLine.h"
#include "BWResult.h"
#include "BWResultLine.h"
#include "BWPattern.h"
#include "BWState.h"
//[[[end]]]

typedef unsigned long long AddrType;
typedef unsigned int OffsetType;

struct ConfigOld {
private:
  AddrType aVD : 64;

  OffsetType oPat : 32;
  OffsetType oRes : 32;

  /*[[[cog
       for field in extra_config_fields:
         if type(field) is BitReducedField: 
            cog.outl("%s %s : %d;" % (field.ty.ty,field.ty.nm,field.ty.bitwidth))
         elif type(field) is ArrayField:
            cog.outl("%s %s[%d];" % (field.ty.ty,field.ty.nm,field.ty.count))
         else:
            cog.outl("%s %s;" % (field.ty,field.nm))
    ]]]*/
  unsigned int nPat : 32;
  unsigned int end_pos : 32;
  unsigned int u0 : 32;
  unsigned int u1 : 32;
  unsigned int u2 : 32;
  unsigned int u3 : 32;
  //[[[end]]] (checksum: 4bbb96a01619899a4000fa572f1e99e0)  

public:
  AddrType get_aVD() const {
    return aVD;
  }
  OffsetType get_oPat() const {
    return oPat & 0xfffffff8;
  }
  OffsetType get_oCl() const {
    return 0;
  }
  OffsetType get_oPre() const {
    return m() << 6; /* starts after the m CL cachelines */
  }
  OffsetType get_oRes() const {
    return oRes & 0xfffffff8;
  }
  unsigned int get_nPat() const {
    return nPat & 0x0fffffff;
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
  unsigned int get_max_recirculating() const {
    return (1 + (((oPat & 0x7) << 3) | ((oRes & 0x7) << 0)))*4;
  }
  unsigned int get_precomp_len() const {
    return (nPat >> 28) & 0xf;
  }

  void set_aVD( const AddrType& val) {
    aVD = val;
  }
  void set_oPat( const OffsetType& val) {
    assert( !(0x7 & val));
    oPat = val | (oPat & 0x7);
    std::cout << "set_oPat: " << val << " " << oPat << " " << get_oPat() << std::endl;
  }
  void set_oCl( const OffsetType& val) {
    assert( val == 0);
  }
  void set_oRes( const OffsetType& val) {
    assert( !(0x7 & val));
    oRes = val | (oRes & 0x7);
  }
  void set_nPat( unsigned int val) {
    assert( !(0xf0000000 & val));
    nPat = (0xf0000000 & nPat) | val;
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
  void set_max_recirculating( unsigned int val) {
    val /= 4;
    assert( 0 < val);
    assert( val <= 64);
    --val;
    oPat &= 0xfffffff8;
    oPat |= (val >> 3) & 0x7;
    oRes &= 0xfffffff8;
    oRes |= (val >> 0) & 0x7;
  }
  void set_precomp_len( unsigned int val) {
    assert( !(0xfffffff0 & val));
    nPat = (  0x0fffffff & nPat) | (val << 28);
  }

  /*[[[cog
       for p in inps + outs:
         cog.outl("AddrType get%sAddr( unsigned int idx) const {" % p.nm.capitalize())
         cog.outl("  return aVD + get_o%s() + (%s::getBitCnt()/8)*idx;" % (p.nm.capitalize(),p.ty))
         cog.outl("}")
    ]]]*/
  AddrType getPatAddr( unsigned int idx) const {
    return aVD + get_oPat() + (BWPattern::getBitCnt()/8)*idx;
  }
  AddrType getClAddr( unsigned int idx) const {
    return aVD + get_oCl() + (BWCacheLine::getBitCnt()/8)*idx;
  }
  AddrType getPreAddr( unsigned int idx) const {
    return aVD + get_oPre() + (BWResult::getBitCnt()/8)*idx;
  }
  AddrType getResAddr( unsigned int idx) const {
    return aVD + get_oRes() + (BWResultLine::getBitCnt()/8)*idx;
  }
  //[[[end]]] (checksum: da6f8a62f325f5ece379ed86043f89e3)  

  /*[[[cog
       for p in inps + outs:
         cog.outl("unsigned int addr2Idx%s( AddrType addr) const {" % p.nm.capitalize())
         cog.outl("  return (addr - aVD - get_o%s()) / ((%s::getBitCnt()/8));" % (p.nm.capitalize(),p.ty))  
         cog.outl("}")
    ]]]*/
  unsigned int addr2IdxPat( AddrType addr) const {
    return (addr - aVD - get_oPat()) / ((BWPattern::getBitCnt()/8));
  }
  unsigned int addr2IdxCl( AddrType addr) const {
    return (addr - aVD - get_oCl()) / ((BWCacheLine::getBitCnt()/8));
  }
  unsigned int addr2IdxPre( AddrType addr) const {
    return (addr - aVD - get_oPre()) / ((BWResult::getBitCnt()/8));
  }
  unsigned int addr2IdxRes( AddrType addr) const {
    return (addr - aVD - get_oRes()) / ((BWResultLine::getBitCnt()/8));
  }
  //[[[end]]] (checksum: 5f78474172ce33dd07f647392b57e7fe)  

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

  ConfigOld() {
    aVD = 0;
    oPat = 0;
    oRes = 0;
    /*[[[cog
         for field in extra_config_fields:
           cog.outl("%s = 0;" % field.nm)
      ]]]*/
    nPat = 0;
    end_pos = 0;
    u0 = 0;
    u1 = 0;
    u2 = 0;
    u3 = 0;
    //[[[end]]] (checksum: e05e0dd87423436fc9b4a548bf93732a)  
  }
  void copy(ConfigOld &from) {
    aVD = from.aVD;
    oPat = from.oPat;
    oRes = from.oRes;
    /*[[[cog
         for field in extra_config_fields:
           cog.outl("%s = from.%s;" % (field.nm,field.nm))
      ]]]*/
    nPat = from.nPat;
    end_pos = from.end_pos;
    u0 = from.u0;
    u1 = from.u1;
    u2 = from.u2;
    u3 = from.u3;
    //[[[end]]] (checksum: 595d8f8077bb7414ae6305b614af6ceb)  
  }

#if !defined(__AAL_USER__) && !defined(USE_SOFTWARE)
  inline friend void sc_trace(sc_trace_file* tf, const ConfigOld& d,
      const std::string& name) {
  }
#endif

  inline friend std::ostream& operator<<(std::ostream& os, const ConfigOld& d) {
    os << "aVD: " << d.aVD << std::endl;
    os << "oPat: " << d.oPat << std::endl;
    os << "oRes: " << d.oRes << std::endl;
    /*[[[cog
         for field in extra_config_fields:
           cog.outl("os << \"%s: \" << d.%s << std::endl;" % (field.nm,field.nm))
      ]]]*/
    os << "nPat: " << d.nPat << std::endl;
    os << "end_pos: " << d.end_pos << std::endl;
    os << "u0: " << d.u0 << std::endl;
    os << "u1: " << d.u1 << std::endl;
    os << "u2: " << d.u2 << std::endl;
    os << "u3: " << d.u3 << std::endl;
    //[[[end]]] (checksum: 817058005489ef4638469497f782e211)  
    return os;
  }

  inline bool operator==(const ConfigOld& rhs) const {
    bool result = true;
    result = result && (aVD == rhs.aVD);
    result = result && (oPat == rhs.oPat);
    result = result && (oRes == rhs.oRes);
    /*[[[cog
         for field in extra_config_fields:
           cog.outl("result = result && (%s == rhs.%s);" % (field.nm,field.nm))
      ]]]*/
    result = result && (nPat == rhs.nPat);
    result = result && (end_pos == rhs.end_pos);
    result = result && (u0 == rhs.u0);
    result = result && (u1 == rhs.u1);
    result = result && (u2 == rhs.u2);
    result = result && (u3 == rhs.u3);
    //[[[end]]] (checksum: 340c3ae83cf696460dbc200b372d0dc5)  
    return result;
  }
};

#endif
