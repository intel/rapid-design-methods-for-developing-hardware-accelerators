// See LICENSE for license details.
#include "gtest/gtest.h"
#include "hld_alloc.h"

#if defined(USE_SOFTWARE) || defined(FPGA_ACC)
typedef unsigned long long UInt64;
#include "AcclApp.h"
#include "Config.h"
#else
#ifdef KERNEL_TEST
#include "vectoradd_hls_tb.h"
#else
#include "vectoradd_acc_tb.h"
#endif
#endif

TEST(AccelTest, SimpleTest) {

#if defined(FPGA_ACC)
  AcclApp theApp("deadbeef");
#else
  AcclApp theApp;
#endif

  const unsigned int N = Blk::ArrayLength;

  unsigned int n = 1U << 20;

  assert( n % N == 0);

  unsigned int n_blks = n/N;

  unsigned long long sz = 3ULL*n*sizeof(unsigned int);

  if ( theApp.alloc( sz)) {
    unsigned char *WORKSPACE = theApp.m_JointVirt;
    size_t WORKSPACE_SIZE    = theApp.m_JointSize;

    Config config;

    hld_alloc altor((char *)WORKSPACE, WORKSPACE_SIZE);

    Blk *ina_ptr = altor.allocate<Blk>( n_blks);
    Blk *inb_ptr = altor.allocate<Blk>( n_blks);
    Blk *out_ptr = altor.allocate<Blk>( n_blks);

    config.set_aIna( (AddrType) ina_ptr);
    config.set_aInb( (AddrType) inb_ptr);
    config.set_aOut( (AddrType) out_ptr);
    config.set_n( n);

    {
      for( unsigned int ip=0; ip<n_blks; ++ip) {
        for ( unsigned int j=0; j<N; ++j) {
          ina_ptr[ip].words[j] = N*ip+j;
          inb_ptr[ip].words[j] = n - (N*ip+j);
        }
      }
    }

    {
      for( unsigned int ip=0; ip<n_blks; ++ip) {
        for ( unsigned int j=0; j<N; ++j) {
          out_ptr[ip].words[j] = 0xdeadbeefU;
        }
      }
    }

    theApp.compute( &config, sizeof(config));
    theApp.join();


    // check
    unsigned int correct = 0;
    for (unsigned int ip=0; ip<n_blks; ++ip) {
      for (unsigned int j=0; j<N; ++j) {
        Blk::ElementType cand = out_ptr[ip].words[j];
        Blk::ElementType ref  = n;

        if ( cand == 0xdeadbeefU) {
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
              << " of " << N*n_blks << " correct." << std::endl;

    EXPECT_EQ( correct, N*n_blks);
    theApp.free();
  } else {
    EXPECT_TRUE( 0);
  }
}

int main (int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
