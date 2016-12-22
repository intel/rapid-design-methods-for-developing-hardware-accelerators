// See LICENSE for license details.
#ifndef __FPGA_APP_IF__
#define __FPGA_APP_IF__

class IFpgaApp {
   virtual void *alloc( unsigned long long size_in_bytes) = 0;  // Should return NULL on failure
   virtual void  compute( const void * config_ptr, const unsigned int config_size) = 0;
   virtual void  join() = 0;
   virtual void  free() = 0;
};

#endif //__FPGA_APP_IF__
