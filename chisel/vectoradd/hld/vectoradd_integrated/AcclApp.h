// See LICENSE for license details.

#ifndef __ACCLAPP_H__
#define __ACCLAPP_H__

#include "opae_svc_wrapper.h"
#include "csr_mgr.h"

#include <string.h>
#include <cassert>

#include "fpga_app_if.h"

#ifndef LOG2_CL
# define LOG2_CL                   6
#endif // LOG2_CL

#ifndef CL
# define CL(x)                     ((x) << LOG2_CL)
#endif // CL

#ifndef MB
# define MB(x)                     ((x) * 1024 * 1024)
#endif // MB

#define LPBK1_DSM_SIZE           MB(4)

#define CSR_CTL                  0x0138
#define CSR_CFG                  0x0140
#define DSM_STATUS_TEST_COMPLETE 0x40
#define CSR_AFU_DSM_BASEL        0x0110
#define CSR_AFU_DSM_BASEH        0x0114

#define CSR_GENERIC_CONFIG       0x0180

#define NLB_TEST_MODE_LPBK1     0x000
#define CSR_STATUS0             0x0160
#define CSR_STATUS1             0x0168

class AcclApp: public IFpgaApp
{
public:

   AcclApp(const char *auid);
   ~AcclApp();

  /* Usage:
   {
     btInt rc = alloc();
     if ( rc == 0) {
       <set up>
       compute( ...);
       <do concurrent work>
       join();
       <check>
       free();
     }
     return rc;
   }
   */
  
   void *alloc( unsigned long long size_in_bytes);  // Returns NULL on failure
   void  compute( const void * config_ptr, const unsigned int config_size);
   void  join();
   void  free();


protected:
   OPAE_SVC_WRAPPER *fpgaPtr;
   CSR_MGR *csrsPtr;
   volatile char *buf, *dsm;
   unsigned long long buf_size;
   unsigned int dsm_size;
   fpga::types::shared_buffer::ptr_t *buf_handlePtr, *dsm_handlePtr;
};

#endif
