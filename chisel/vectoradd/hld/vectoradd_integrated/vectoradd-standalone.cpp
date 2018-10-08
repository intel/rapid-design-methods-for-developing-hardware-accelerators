//
// Copyright (c) 2017, Intel Corporation
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
//
// Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// Neither the name of the Intel Corporation nor the names of its contributors
// may be used to endorse or promote products derived from this software
// without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include <iostream>
#include <string>
#include <atomic>

using namespace std;

#include "opae_svc_wrapper.h"
#include "csr_mgr.h"

#include "opae/mmio.h"


using namespace opae::fpga::types;
using namespace opae::fpga::bbb::mpf::types;

// State from the AFU's JSON file, extracted using OPAE's afu_json_mgr script
#include "afu_json_info.h"

struct Config {
  unsigned long long in0Addr : 64;
  unsigned long long in1Addr : 64;
  unsigned long long outAddr : 64;
  unsigned vecCount : 32;
};

//
// Construct a linked list of type t_linked_list in a buffer starting at
// head.  Generated the list with n_entries, separating each entry by
// spacing_bytes.
//
// Both head and spacing_bytes must be cache-line aligned.
//
void init( unsigned int *a, unsigned int *b, unsigned int *c, int count)
{
  for ( unsigned int i = 0; i<count; ++i) {
    a[i] = 0;
    b[i] = i;
    c[i] = 0xdeadbeef;
  }

  // Force all initialization to memory before the buffer is passed to the FPGA.
  std::atomic_thread_fence(std::memory_order_seq_cst);
}

void test( const unsigned int * const a,
           const unsigned int * const b,
           const unsigned int * const c, int count) {
  unsigned int failures = 0;

  for ( unsigned int i = 0; i<count; ++i) {
    unsigned int sum = a[i] + b[i];
    if ( sum != c[i]) {
      ++failures;
    }
  }

  std::cout << "Number of failures: " << failures << std::endl;
}

#define DSM

int main(int argc, char *argv[])
{
    // Find and connect to the accelerator
    OPAE_SVC_WRAPPER fpga(AFU_ACCEL_UUID);
    assert(fpga.isOk());

    // Connect the CSR manager
    CSR_MGR csrs(fpga);

    size_t count = 16*1024;
    size_t workspace_size = 3*count*sizeof(unsigned int) ;
  
    // Allocate a buffer large enough for three vectors
    // and share it with the FPGA.  Because the FPGA
    // is using VTP we can allocate a virtually contiguous region.
    // OPAE_SVC_WRAPPER detects the presence of VTP and uses it for memory
    // allocation instead of calling OPAE directly.  The buffer will
    // be composed of physically discontiguous pages.  VTP will construct
    // a private TLB to map virtual addresses from this process to FPGA-side
    // physical addresses.

    auto buf_handle = fpga.allocBuffer( workspace_size);

    assert(buf_handle->c_type());

    volatile char* buf = reinterpret_cast<volatile char*>(buf_handle->c_type());

    std::cout << "BUF ALLOCATED OF SIZE " << dec << workspace_size << " @ " << hex << reinterpret_cast<uint64_t>(buf) << std::endl;

    std::cout << dec;
    
#ifdef DSM
    const unsigned int dsm_size = 4*1024;
    auto dsm_handle = fpga.allocBuffer( dsm_size);

    assert(dsm_handle->c_type());
    volatile char* dsm = reinterpret_cast<volatile char*>(dsm_handle->c_type());

    std::cout << "DSM SPACE ALLOCATED OF SIZE " << dec << dsm_size << " @ " << hex << reinterpret_cast<uint64_t>(dsm) << std::endl;
    std::cout << dec;

    ::memset( const_cast<char*>( dsm), 0, dsm_size);
#endif

    unsigned int *a = reinterpret_cast<unsigned int*>(const_cast<char*>(buf) + 0
*count*sizeof(unsigned int));
    unsigned int *b = reinterpret_cast<unsigned int*>(const_cast<char*>(buf) + 1*count*sizeof(unsigned int));
    unsigned int *c = reinterpret_cast<unsigned int*>(const_cast<char*>(buf) + 2*count*sizeof(unsigned int));

    init( a, b, c, count);
    test( a, b, c, count);

#ifdef DSM
    const unsigned int CSR_AFU_DSM_BASEL = 0x0110;
    const unsigned int CSR_AFU_DSM_BASEH = 0x0114;

    fpga.write_csr64( CSR_AFU_DSM_BASEL, reinterpret_cast<uint64_t>(dsm));

    std::cout << "Wrote to CSR_AFU_DSM_BASE{L,H}" << std::endl;
#endif

    const uint32_t CSR_CTL = 0x138;
    const uint32_t CSR_CFG = 0x140;
    const uint32_t CSR_GENERIC_CONFIG = 0x180;

    fpga.write_csr64( CSR_CTL, 0);

    fpga.write_csr64( CSR_CTL, 1);

    Config config;

    config.in0Addr = reinterpret_cast<unsigned long long>( a);
    config.in1Addr = reinterpret_cast<unsigned long long>( b);
    config.outAddr = reinterpret_cast<unsigned long long>( c);
    config.vecCount = count/16;

    Config* config_ptr = &config;

    {
      const uint32_t num_of_ulls_in_config = (sizeof(struct Config)+7)/8;
      assert( num_of_ulls_in_config <= 9);
      const uint64_t *config_as_ull = reinterpret_cast<const uint64_t*>( config_ptr);
      for ( uint32_t i=0; i < num_of_ulls_in_config; ++i) {
        fpga.write_csr64( CSR_GENERIC_CONFIG+i*8, config_as_ull[i]);
      }
    }
    std::cout << "Wrote to CSR_GENERIC_CONFIG" << std::endl;

    fpga.write_csr64( CSR_CFG, 0);
    fpga.write_csr64( CSR_CTL, 3);



    // Spin, waiting for the value in memory to change to something non-zero.
    struct timespec pause;
    // Longer when simulating
    if ( fpga.hwIsSimulated()) {
      pause.tv_sec = 1;
      pause.tv_nsec = 0;
    } else {
      pause.tv_sec = 0;
      pause.tv_nsec = 2500000;
    }


#ifdef DSM
    // look of DSM_STATUS_TEST_COMPLETE bit of m_DSMVirt
    const unsigned int DSM_STATUS_TEST_COMPLETE = 0x40;

    volatile unsigned int *StatusAddr = reinterpret_cast<volatile unsigned int*>( dsm + DSM_STATUS_TEST_COMPLETE);

    while ( 0 == ((*StatusAddr)&0x1)) 
#else
    for ( unsigned int i=0; i< 10; ++i)
#endif
    {
      std::cout << "Spin loop" << std::endl;
      nanosleep(&pause, NULL);
    }

    test( a, b, c, count);

    fpga.write_csr64( CSR_CTL, 7);

#if 1
    cout << "#" << endl
         << "# AFU frequency: " << csrs.getAFUMHz() << " MHz"
         << (fpga.hwIsSimulated() ? " [simulated]" : "")
         << endl;

    // MPF VTP (virtual to physical) statistics
    mpf_handle::ptr_t mpf = fpga.mpf;
    if (mpfVtpIsAvailable(*mpf))
    {
        mpf_vtp_stats vtp_stats;
        mpfVtpGetStats(*mpf, &vtp_stats);

        cout << "#" << endl;
        if (vtp_stats.numFailedTranslations)
        {
            cout << "# VTP failed translating VA: 0x" << hex << uint64_t(vtp_stats.ptWalkLastVAddr) << dec << endl;
        }
        cout << "# VTP PT walk cycles: " << vtp_stats.numPTWalkBusyCycles << endl
             << "# VTP L2 4KB hit / miss: " << vtp_stats.numTLBHits4KB << " / "
             << vtp_stats.numTLBMisses4KB << endl
             << "# VTP L2 2MB hit / miss: " << vtp_stats.numTLBHits2MB << " / "
             << vtp_stats.numTLBMisses2MB << endl;
    }
#endif


    // All shared buffers are automatically released and the FPGA connection
    // is closed when their destructors are invoked here.
    return 0;
}
