// See LICENSE for license details.
#include "CacheLine.h"

typedef unsigned long long AddrType;
typedef unsigned int OffsetType;

struct Config {
private:
  AddrType aInp : 64;
  AddrType aOut : 64;
  unsigned int nCLs : 32;
public:
  AddrType get_aInp() const { return aInp; }
  AddrType get_aOut() const { return aOut; }
  unsigned int get_nCLs() const { return nCLs; }

  void set_aInp( AddrType val) { aInp = val; }
  void set_aOut( AddrType val) { aOut = val; }
  void set_nCLs( unsigned int val) { nCLs = val; }

  CacheLine *getInpPtr() const {
    return reinterpret_cast<CacheLine*>( get_aInp());
  }
  CacheLine *getOutPtr() const {
    return reinterpret_cast<CacheLine*>( get_aOut());
  }

  Config() {
    aInp = aOut = nCLs = 0;
  }
};
