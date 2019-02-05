// See LICENSE for license details.
#include "CacheLine.h"

typedef unsigned long long AddrType;
typedef unsigned int OffsetType;

struct Config {
private:
  AddrType aGi  : 64;
  AddrType aWi  : 64;
  AddrType aInp : 64;
  AddrType aExp : 64;
  AddrType aMi  : 64;
  AddrType aMo  : 64;
  unsigned int nCLperRow : 32; // should be 3 if row size is 96
  unsigned int nSlices : 32; // n*(n-1)/2
public:
  AddrType get_aGi() const { return aGi; }
  AddrType get_aWi() const { return aWi; }
  AddrType get_aInp() const { return aInp; }
  AddrType get_aExp() const { return aExp; }
  AddrType get_aMi() const { return aMi; }
  AddrType get_aMo() const { return aMo; }
  unsigned int get_nCLperRow() const { return nCLperRow; }
  unsigned int get_nSlices() const { return nSlices; }

  void set_aGi( AddrType val) { aGi = val; }
  void set_aWi( AddrType val) { aWi = val; }
  void set_aInp( AddrType val) { aInp = val; }
  void set_aExp( AddrType val) { aExp = val; }
  void set_aMi( AddrType val) { aMi = val; }
  void set_aMo( AddrType val) { aMo = val; }
  void set_nCLperRow( unsigned int val) { nCLperRow = val; }
  void set_nSlices( unsigned int val) { nSlices = val; }

  UCacheLine *getGiPtr() const { return reinterpret_cast<UCacheLine*>( get_aGi()); }
  CacheLine *getWiPtr() const { return reinterpret_cast<CacheLine*>( get_aWi()); }
  CacheLine *getInpPtr() const { return reinterpret_cast<CacheLine*>( get_aInp()); }
  UCacheLine *getExpPtr() const { return reinterpret_cast<UCacheLine*>( get_aExp()); }
  UCacheLine *getMiPtr() const { return reinterpret_cast<UCacheLine*>( get_aMi()); }
  UCacheLine *getMoPtr() const { return reinterpret_cast<UCacheLine*>( get_aMo()); }

  Config() {
    aGi = aWi = aMi = aMo = nCLperRow = nSlices = 0;
  }
};
