// See LICENSE for license details.

#include "AcclApp.h"
typedef unsigned long long UInt64;
#include "Config.h"

#include <string.h>

void AcclApp::compute( const void *config_void_ptr, const unsigned int config_size) {
  const Config &config = *static_cast<const Config *>(config_void_ptr);

  const CacheLine* inp_ptr = config.getInpPtr();
  CacheLine* out_ptr = config.getOutPtr();

  memcpy( out_ptr, inp_ptr, sizeof(CacheLine)*config.get_nCLs());
}
