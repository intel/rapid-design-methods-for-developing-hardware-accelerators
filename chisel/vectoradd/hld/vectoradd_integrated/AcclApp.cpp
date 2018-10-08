// See LICENSE for license details.
using namespace std;

#include <iostream>
#include <ios>

#include "AcclApp.h"

using namespace opae::fpga::types;
using namespace opae::fpga::bbb::mpf::types;

AcclApp::AcclApp(const char *auid) :
   fpgaPtr(0),
   csrsPtr(0),
   buf(0),
   dsm(0),
   buf_size(0ULL),
   dsm_size(0U),
   buf_handlePtr(0),
   dsm_handlePtr(0)
{

  fpgaPtr = new OPAE_SVC_WRAPPER( auid);
  assert( fpgaPtr);
  assert( fpgaPtr->isOk());

  csrsPtr = new CSR_MGR(*fpgaPtr);
}

/// @brief   Destructor
///
AcclApp::~AcclApp()
{
  delete dsm_handlePtr;
  delete buf_handlePtr;
  delete csrsPtr;
  delete fpgaPtr;
}

void *AcclApp::alloc( unsigned long long size_in_bytes)
{
  buf_size = size_in_bytes;
  dsm_size = 4*1024;

  buf_handlePtr = new fpga::types::shared_buffer::ptr_t(fpgaPtr->allocBuffer( size_in_bytes));
  assert( (*buf_handlePtr)->c_type());
  buf = reinterpret_cast<volatile char*>((*buf_handlePtr)->c_type());

  std::cout << "BUF ALLOCATED OF SIZE " << dec << size_in_bytes << " @ " << hex << reinterpret_cast<uint64_t>(buf) << dec << std::endl;

  dsm_handlePtr = new fpga::types::shared_buffer::ptr_t(fpgaPtr->allocBuffer( dsm_size));
  assert((*dsm_handlePtr)->c_type());
  dsm = reinterpret_cast<volatile char*>((*dsm_handlePtr)->c_type());

  std::cout << "DSM SPACE ALLOCATED OF SIZE " << dec << dsm_size << " @ " << hex << reinterpret_cast<uint64_t>(dsm) << dec << std::endl;

  ::memset( const_cast<char*>( dsm), 0, dsm_size);

  fpgaPtr->write_csr64( CSR_AFU_DSM_BASEL, reinterpret_cast<uint64_t>(dsm));

  return const_cast<void *>( reinterpret_cast<volatile void*>( buf));
}

void AcclApp::compute( const void *config_ptr, const unsigned int config_size) 
{
  ::memset( const_cast<char*>( dsm), 0, dsm_size);

  fpgaPtr->write_csr64( CSR_CTL, 0);
  fpgaPtr->write_csr64( CSR_CTL, 1);

  {
    const uint32_t num_of_ulls_in_config = (config_size+7)/8;
    assert( num_of_ulls_in_config <= 9);
    const uint64_t *config_as_ull =
      reinterpret_cast<const uint64_t*>( config_ptr);
    for ( uint32_t i=0; i < num_of_ulls_in_config; ++i) {
      fpgaPtr->write_csr64( CSR_GENERIC_CONFIG+i*8, config_as_ull[i]);
    }
  }
  std::cout << "Wrote to CSR_GENERIC_CONFIG" << std::endl;

  fpgaPtr->write_csr64( CSR_CFG, 0);
  fpgaPtr->write_csr64( CSR_CTL, 3);
}

void AcclApp::join() 
{
  struct timespec pause;
  // Longer when simulating
  if ( fpgaPtr->hwIsSimulated()) {
    pause.tv_sec = 1;
    pause.tv_nsec = 0;
  } else {
    pause.tv_sec = 0;
    pause.tv_nsec = 2500000;
  }

  volatile unsigned int *StatusAddr = reinterpret_cast<volatile unsigned int*>
( dsm + DSM_STATUS_TEST_COMPLETE);

  while ( 0 == ((*StatusAddr)&0x1)) {
    std::cout << "Spin loop" << std::endl;
    nanosleep(&pause, NULL);
  }

  fpgaPtr->write_csr64( CSR_CTL, 7);

  {
    volatile unsigned long long *q = reinterpret_cast<volatile unsigned long long *>(reinterpret_cast<volatile unsigned char *>(StatusAddr)+8);
    // Forty bits defined
    std::cout << "DSM: Num_ticks: " << (0xffffffffff & *q) << std::endl;
  }

  {
    volatile unsigned int *q = reinterpret_cast<volatile unsigned int *>(reinterpret_cast<volatile unsigned char *>(StatusAddr)+16);
    std::cout << "DSM: Num_Reads: " << *q << std::endl;
  }
  {
    volatile unsigned int *q = reinterpret_cast<volatile unsigned int *>(reinterpret_cast<volatile unsigned char *>(StatusAddr)+20);
    std::cout << "DSM: Num_Writes: " << *q << std::endl;
  }
      
  unsigned int wr_ticks = fpgaPtr->read_csr32(CSR_STATUS0);
  unsigned int rd_ticks = fpgaPtr->read_csr32(CSR_STATUS0+4);
  std::cout << "CSR: Num rd_ticks: " << rd_ticks << std::endl;
  std::cout << "CSR: Num wr_ticks: " << wr_ticks << std::endl;

  unsigned int afu_idle = fpgaPtr->read_csr32(CSR_STATUS1);
  unsigned int ivp_idle = fpgaPtr->read_csr32(CSR_STATUS1+4);  
  std::cout << "CSR: Afu idle cycles: " << afu_idle << std::endl;
  std::cout << "CSR: Ivp idle cycles: " << ivp_idle << std::endl;

#if 1
  cout << "#" << endl
       << "# AFU frequency: " << csrsPtr->getAFUMHz() << " MHz"
       << (fpgaPtr->hwIsSimulated() ? " [simulated]" : "")
       << endl;

  // MPF VTP (virtual to physical) statistics
  mpf_handle::ptr_t mpf = fpgaPtr->mpf;
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

}

void AcclApp::free() 
{
  cout << "Done Running Test";
}

