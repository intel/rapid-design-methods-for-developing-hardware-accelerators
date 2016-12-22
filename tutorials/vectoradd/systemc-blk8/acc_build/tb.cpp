// See LICENSE for license details.
// See LICENSE for license details.
#include "gtest/gtest.h"
#include "hld_alloc.h"

#ifdef USE_SOFTWARE
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

  AcclApp theApp;

  unsigned int n = 16*128*1024;

  assert( n % 8 == 0);

  unsigned int n_blks = n/8;

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
        for ( unsigned int j=0; j<8; ++j) {
          ina_ptr[ip].words[j] = 8*ip+j;
          inb_ptr[ip].words[j] = n - (8*ip+j);
        }
      }
    }

    {
      for( unsigned int ip=0; ip<n_blks; ++ip) {
        for ( unsigned int j=0; j<8; ++j) {
          out_ptr[ip].words[j] = 0xdeadbeefU;
        }
      }
    }

    theApp.compute( &config, sizeof(config));
    theApp.join();


    // check
    unsigned int correct = 0;
    for (unsigned int ip=0; ip<n_blks; ++ip) {
      for (unsigned int j=0; j<8; ++j) {
        unsigned long long cand = out_ptr[ip].words[j];
        unsigned long long ref  = n;
        if ( cand == 0xdeadbeefULL) {
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
              << " of " << 8*n_blks << " correct." << std::endl;

    EXPECT_EQ( correct, 8*n_blks);
    theApp.free();
  } else {
    EXPECT_TRUE( 0);
  }
}

int main (int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
