// See LICENSE for license details.

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

#include "Node.h"
#include "CycleExistInfo.h"

typedef unsigned long long AddrType;
typedef unsigned int OffsetType;

struct Config {
private:
  AddrType aInp : 64;
  AddrType aOut : 64;

public:
  AddrType get_aInp() const {
    return aInp & 0x0000ffffffffffc0ULL;
  }
  AddrType get_aOut() const {
    return aOut & 0x0000ffffffffffc0ULL;
  }
  void set_aInp( const AddrType& val) {
    assert( !(val & ~0x0000ffffffffffc0ULL));
    aInp = val;
  }
  void set_aOut( const AddrType& val) {
    assert( !(val & ~0x0000ffffffffffc0ULL));
    aOut = val;
  }

  Node* getInpPtr() const {
    return reinterpret_cast<Node*>( get_aInp());
  }
  CycleExistInfo* getOutPtr() const {
    return reinterpret_cast<CycleExistInfo*>( get_aOut());
  }
};
#endif
