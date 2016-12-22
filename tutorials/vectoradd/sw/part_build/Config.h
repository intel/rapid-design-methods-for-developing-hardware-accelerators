// See LICENSE for license details.
#include "Blk.h"

typedef unsigned long long AddrType;

struct Config {
private:
  AddrType aIna : 64;
  AddrType aInb : 64;
  AddrType aOut : 64;
  unsigned int n : 32;
public:
  AddrType get_aIna() const { return aIna; }
  AddrType get_aInb() const { return aInb; }
  AddrType get_aOut() const { return aOut; }
  unsigned int get_n() const { return n; }

  void set_aIna( AddrType val) { aIna = val; }
  void set_aInb( AddrType val) { aInb = val; }
  void set_aOut( AddrType val) { aOut = val; }
  void set_n( unsigned int val) { n = val; }

  Blk *getInaPtr() const {
    return reinterpret_cast<Blk*>( get_aIna());
  }
  Blk *getInbPtr() const {
    return reinterpret_cast<Blk*>( get_aInb());
  }
  Blk *getOutPtr() const {
    return reinterpret_cast<Blk*>( get_aOut());
  }

  Config() {
    aIna = aInb = aOut = n = 0;
  }
};
