// See LICENSE for license details.

#include "gtest/gtest.h"
#include "hld_alloc.h"

#ifndef WORDS_PER_BLK
#define WORDS_PER_BLK 32
#endif

#if defined(USE_SOFTWARE) || defined(__AAL_USER__)
#define PATH_PREFIX "../"
#ifndef NUM_AUS
#define NUM_AUS 4
#endif
typedef unsigned long long UInt64;
#include "Config.h"
#include "AcclApp.h"
#else
#define PATH_PREFIX ""
#ifdef KERNEL_TEST
#include "sobel_hls_tb.h"
#else
#include "sobel_acc_tb.h"
#endif
#endif

#define BIGGEST_TEST

#ifdef BIGGEST_TEST
const int ni = 1600;
const int words_per_blk = WORDS_PER_BLK;
const int bpr = 2560 / WORDS_PER_BLK;

const char * a_fn = PATH_PREFIX "../data/ronaldo_a_1600_2560.txt";
const char * r_fn = PATH_PREFIX "../data/ronaldo_r_1600_2560.txt";
#else
#ifdef BIGGER_TEST
const int ni = 342;
const int words_per_blk = WORDS_PER_BLK;
const int bpr = 512 / WORDS_PER_BLK;

const char * a_fn = PATH_PREFIX "../data/messi_a_342_512.txt";
const char * r_fn = PATH_PREFIX "../data/messi_r_342_512.txt";
#else
const int ni = 192;
const int words_per_blk = WORDS_PER_BLK;
const int bpr = 192 / WORDS_PER_BLK;

const char * a_fn = PATH_PREFIX "../data/fe_in.txt";
const char * r_fn = PATH_PREFIX "../data/re_out.txt";
#endif
#endif

typedef BlkInp inp_t;
typedef BlkOut out_t;

out_t refa[bpr*ni];

unsigned char *WORKSPACE;
size_t WORKSPACE_SIZE;

TEST(AccelTest, SimpleTest) {

#ifdef __AAL_USER__
  AcclApp theApp("564b3e10b57f347dbbec808025f58c55e");
#else
#ifdef USE_KERNEL
  AcclApp theApp("theApp");
#else
  AcclApp theApp;
#endif
#endif

  unsigned long long sz = 1024ULL * 1024ULL * 1024ULL;

  if ( theApp.alloc( sz)) {
    unsigned char *WORKSPACE = theApp.m_JointVirt;
    size_t WORKSPACE_SIZE    = theApp.m_JointSize;

    Config config;

    hld_alloc altor((char *)WORKSPACE,WORKSPACE_SIZE);

    unsigned int nimages = NUM_AUS;

    inp_t *inp_ptr = altor.allocate<inp_t>( ni*bpr*nimages);
    out_t *out_ptr = altor.allocate<out_t>( ni*bpr*nimages);

    config.set_aInp( (AddrType) inp_ptr);
    config.set_aOut( (AddrType) out_ptr);

    config.set_nInp( ni*bpr);
    config.set_num_of_rows( ni);
    config.set_row_size_in_blks( bpr);
    config.set_image_size_in_bytes( ni*bpr*words_per_blk);

    config.set_num_of_images( nimages); 

    std::cout << "Reading from " << a_fn << " and " << r_fn << "." << std::endl;
    std::cout << "nimages, ni, words_per_blk, bpr:" << nimages << " " << ni << " " << words_per_blk << " " << bpr << std::endl;

    {
      std::ifstream ifp;
      ifp.open( a_fn);
      assert(ifp.is_open());
      for (int ip=0; ip<ni; ++ip) {
        for (int jc=0; jc<bpr; ++jc) {
          for (int k=0; k<words_per_blk; ++k) {
            unsigned int x;
            ifp >> x;
            inp_ptr[ip*bpr+jc].data[k] = x;
          }
        }
      }
    }
    {
      for( unsigned int q=1; q<nimages; ++q) {
        for (int ip=0; ip<ni; ++ip) {
          for (int jc=0; jc<bpr; ++jc) {
              inp_ptr[(q*ni+ip)*bpr+jc] = inp_ptr[ip*bpr+jc];
          }
        }
      }
    }

    {
      std::ifstream ifp;
      ifp.open( r_fn);
      assert(ifp.is_open());

      for (int ip=0; ip<ni; ++ip) {
        for (int jc=0; jc<bpr; ++jc) {
          for (int k=0; k<words_per_blk; ++k) {
            int x;
            ifp >> x;
            refa[ip*bpr+jc].data[k] = x;
          }
        }
      }
    }


    theApp.compute( &config, sizeof(config));
    theApp.join();

    int count_failures = 0;
    for( unsigned int q=0; q<nimages; ++q) {
      for (int ip=0; ip<ni; ++ip) {
        for (int jc=0; jc<bpr; ++jc) {
          if ( refa[ip*bpr+jc] == out_ptr[(q*ni+ip)*bpr+jc]) {
          } else {
            ++count_failures;
            for (int k=0; k<words_per_blk; ++k) {
              if ( refa[ip*bpr+jc].data[k] != out_ptr[(q*ni+ip)*bpr+jc].data[k]) {
                std::cout << q << "," << ip << "," << jc << "," << k << ": " << (int) refa[ip*bpr+jc].data[k] << " " << (int) out_ptr[q*ni*bpr+ip*bpr+jc].data[k] << std::endl;
              }
            }
          }
        }
      }
    }
    std::cout << "Number of differences: " << count_failures << std::endl;

    ASSERT_EQ( count_failures, 0);
    theApp.free();
  } else {
    ASSERT_TRUE( 0);
  }

}

#ifdef __AAL_USER__
int main(int argc, char *argv[]) {
#else
int sc_main(int argc, char *argv[]) {
  sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", SC_DO_NOTHING);
#endif
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
