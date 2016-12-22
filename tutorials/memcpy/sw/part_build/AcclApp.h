// See LICENSE for license details.
#include "fpga_app_sw.h"

struct AcclApp : public FpgaAppSwAlloc {
   void compute (const void * config_void_ptr, const unsigned int config_size);
   void join() {}
};
