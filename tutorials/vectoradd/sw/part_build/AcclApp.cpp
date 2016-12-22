// See LICENSE for license details.

#include "AcclApp.h"
typedef unsigned long long UInt64;
#include "Config.h"

#include <string.h>

void AcclApp::compute( const void *config_void_ptr, const unsigned int config_size) {
  const Config &config = *static_cast<const Config *>(config_void_ptr);

  const Blk* ina_ptr = config.getInaPtr();
  const Blk* inb_ptr = config.getInbPtr();
  Blk* out_ptr = config.getOutPtr();

  unsigned int n_blks = config.get_n()/16;

  for( unsigned int ip=0; ip<n_blks; ++ip) {
    for( unsigned int j=0; j<16; ++j) {
      out_ptr[ip].words[j] = ina_ptr[ip].words[j] + inb_ptr[ip].words[j];
    }
  }
}
