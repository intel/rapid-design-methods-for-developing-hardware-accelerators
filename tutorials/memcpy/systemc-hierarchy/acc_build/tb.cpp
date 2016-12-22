// See LICENSE for license details.
// See LICENSE for license details.
#include "gtest/gtest.h"
#include "hld_alloc.h"

#if defined(USE_SOFTWARE) || defined(__AAL_USER__)
typedef unsigned long long UInt64;
#include "AcclApp.h"
#else
#ifdef KERNEL_TEST
#include "memcpy_hls_tb.h"
#else
#include "memcpy_acc_tb.h"
#endif
#endif

#include "Config.h"

TEST(AccelTest, SimpleTest) {

#if defined(__AAL_USER__)
  AcclApp theApp("deadbeef");
#else
  AcclApp theApp;
#endif

  unsigned int n_cls = 64*1024;

  unsigned long long sz = 2ULL*n_cls*64;

  if ( theApp.alloc( sz)) {
    unsigned char *WORKSPACE = theApp.m_JointVirt;
    size_t WORKSPACE_SIZE    = theApp.m_JointSize;

    Config config;

    hld_alloc altor((char *)WORKSPACE, WORKSPACE_SIZE);

    CacheLine *inp_ptr = altor.allocate<CacheLine>( n_cls);
    CacheLine *out_ptr = altor.allocate<CacheLine>( n_cls);

    config.set_aInp( (AddrType) inp_ptr);
    config.set_aOut( (AddrType) out_ptr);
    config.set_nCLs( n_cls);

    {
      for( unsigned int ip=0; ip<n_cls; ++ip) {
        for ( unsigned int j=0; j<8; ++j) {
          inp_ptr[ip].words[j] = 8*ip+j;
        }
      }
    }

    {
      for( unsigned int ip=0; ip<n_cls; ++ip) {
        for ( unsigned int j=0; j<8; ++j) {
          out_ptr[ip].words[j] = 0xdeadbeefdeadbeefULL;
        }
      }
    }

    theApp.compute( &config, sizeof(config));
    theApp.join();


    // check
    unsigned int correct = 0;
    for (unsigned int ip=0; ip<n_cls; ++ip) {
      for (unsigned int j=0; j<8; ++j) {
        unsigned long long cand = out_ptr[ip].words[j];
        unsigned long long ref  = 8*ip+j;
        if ( cand == 0xdeadbeefdeadbeefULL) {
          std::cout << "Uninitialized result at " << ip << "," << j << std::endl;
        }

        if ( cand == ref) {
          ++correct;
        } else {
          std::cout << ip << "," << j << " != " << ref << " " << cand << std::endl;
        }
      }
    }
    std::cout << "Results checked. " << correct
              << " of " << 8*n_cls << " correct." << std::endl;

    EXPECT_EQ( correct, 8*n_cls);
    theApp.free();
  } else {
    EXPECT_TRUE( 0);
  }
}

int main (int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
