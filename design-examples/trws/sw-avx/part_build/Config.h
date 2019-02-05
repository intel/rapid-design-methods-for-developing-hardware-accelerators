// See LICENSE for license details.

typedef unsigned long long AddrType;
typedef unsigned int OffsetType;

struct Pair {
  unsigned short a;
  unsigned short e;
};


struct Config {
private:
  AddrType aGi  : 64;
  AddrType aMi  : 64;
  AddrType aWi  : 64;
  AddrType aMo  : 64;
  AddrType aSlc : 64;
  AddrType aLof : 64;

  unsigned int nCLperRow : 32; // should be 3 if row size is 96
  unsigned int nSlices : 32; // n*(n-1)/2
  unsigned int modeLoad : 1;
  unsigned int modeCompute : 1;
  unsigned int loadIdx : 1;
  unsigned int computeIdx : 1;
  unsigned int dummy : 28;
  unsigned int dummy2 : 32;
public:
  AddrType get_aGi() const { return aGi; }
  AddrType get_aWi() const { return aWi; }
  AddrType get_aSlc() const { return aSlc; }
  AddrType get_aLof() const { return aLof; }
  AddrType get_aMi() const { return aMi; }
  AddrType get_aMo() const { return aMo; }
  unsigned int get_nCLperRow() const { return nCLperRow; }
  unsigned int get_nSlices() const { return nSlices; }
  bool get_modeLoad() const { return modeLoad; }
  bool get_modeCompute() const { return modeCompute; }
  unsigned int get_loadIdx() const { return loadIdx; }
  unsigned int get_computeIdx() const { return computeIdx; }

  void set_aGi( AddrType val) { aGi = val; }
  void set_aWi( AddrType val) { aWi = val; }
  void set_aSlc( AddrType val) { aSlc = val; }
  void set_aLof( AddrType val) { aLof = val; }
  void set_aMi( AddrType val) { aMi = val; }
  void set_aMo( AddrType val) { aMo = val; }
  void set_nCLperRow( unsigned int val) { nCLperRow = val; }
  void set_nSlices( unsigned int val) { nSlices = val; }
  void set_modeLoad( bool val) { modeLoad = val; }
  void set_modeCompute( bool val) { modeCompute = val; }
  void set_loadIdx( unsigned int val) { loadIdx = val; }
  void set_computeIdx( unsigned int val) { computeIdx = val; }

  unsigned short *getGiPtr() const { return reinterpret_cast<unsigned short*>( get_aGi()); }
  short *getWiPtr() const { return reinterpret_cast<short*>( get_aWi()); }
  Pair *getLofPtr() const { return reinterpret_cast<Pair*>( get_aLof()); }
  Pair *getSlcPtr() const { return reinterpret_cast<Pair*>( get_aSlc()); }
  unsigned short *getMiPtr() const { return reinterpret_cast<unsigned short*>( get_aMi()); }
  unsigned short *getMoPtr() const { return reinterpret_cast<unsigned short*>( get_aMo()); }

  Config() {
    aGi = aWi = aMi = aMo = aSlc = aLof = nCLperRow = nSlices = 0;
    modeLoad = modeCompute = loadIdx = computeIdx = 0;
  }
};
