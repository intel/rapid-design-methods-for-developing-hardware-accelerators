// See LICENSE for license details.
#include "gtest/gtest.h"
#include "hld_alloc.h"

#define USE_TBB
#ifdef USE_TBB
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/tick_count.h>
#include <tbb/task_scheduler_init.h>
#endif


#if defined(USE_SOFTWARE) || defined(__AAL_USER__)
typedef unsigned long long UInt64;
#include "AcclApp.h"
#else
#ifdef KERNEL_TEST
#include "trws_hls_tb.h"
#else
#include "trws_acc_tb.h"
#endif
#endif

#include "Config.h"

#include <random>

unsigned long long next64( unsigned long long x) {
  return 64*((x+63)/64);
}

TEST(AccelTest, SimpleTest) {

  std::default_random_engine generator;
  std::uniform_int_distribution<short> dist_short(-(1<<7),(1<<7)-1);
  std::uniform_int_distribution<unsigned short> dist_ushort(0,(1<<8)-1);

#if defined(__AAL_USER__)
  AcclApp theApp("37F06FD9-5290-CE88-F526-5F21A37C24E2");
#else
  AcclApp theApp;
#endif

  const unsigned int nCLperRow = 3;
  const unsigned int elements_per_cl = 32;
  const unsigned int n = elements_per_cl*nCLperRow;
  const unsigned int nSlices = n*(n-1)/2;

  unsigned long long sz = next64( 2*n)
                        + next64( 2*n*n)
                        + next64( 4*n*n)
                        + next64( 4*nSlices)
                        + next64( 2*nSlices*n)
                        + next64( 2*nSlices*n);

  std::cout << "sz: " << sz << std::endl;

  if ( theApp.alloc( sz)) {
    unsigned char *WORKSPACE = theApp.m_JointVirt;
    size_t WORKSPACE_SIZE    = theApp.m_JointSize;

    Config config;

    hld_alloc altor((char *)WORKSPACE, WORKSPACE_SIZE);

    unsigned short *gi_ptr = altor.allocate<unsigned short>( n);
    short *wi_ptr = altor.allocate<short>( n*n);
    Pair *slc_ptr = altor.allocate<Pair>( n*n);
    Pair *lof_ptr = altor.allocate<Pair>( nSlices);

    unsigned short *mi_ptr = altor.allocate<unsigned short>( nSlices*n);
    unsigned short *mo_ptr = altor.allocate<unsigned short>( nSlices*n);

    config.set_aGi( (AddrType) gi_ptr);
    config.set_aWi( (AddrType) wi_ptr);
    config.set_aSlc( (AddrType) slc_ptr);
    config.set_aLof( (AddrType) lof_ptr);
    config.set_aMi( (AddrType) mi_ptr);
    config.set_aMo( (AddrType) mo_ptr);

    config.set_nCLperRow( nCLperRow);
    config.set_nSlices( nSlices);

    assert( nCLperRow == 3);

    {
      for ( unsigned int j=0; j<n; ++j) {
        gi_ptr[j] = dist_ushort( generator);
      }
    }
    {
      for ( unsigned int i=0; i<n; ++i) {
        for ( unsigned int j=0; j<n; ++j) {
          wi_ptr[n*i+j] = dist_short( generator);
        }
      }
    }

    {
      for ( unsigned int i=0; i<n; ++i) {
        for ( unsigned int j=0; j<n; ++j) {
          unsigned short y = dist_ushort( generator);
          unsigned short x = dist_ushort( generator);
          unsigned int idx = n*i+j;
          slc_ptr[idx].a = x;
          slc_ptr[idx].e = y;
        }
      }
      for ( unsigned int k=0; k<nSlices; ++k) {
        lof_ptr[k].a = dist_ushort( generator);
        lof_ptr[k].e = dist_ushort( generator);
      }
    }

    {
      for( unsigned int k=0; k<nSlices; ++k) {
        for ( unsigned int j=0; j<n; ++j) {
          mi_ptr[n*k+j] = dist_ushort( generator);
          mo_ptr[n*k+j] = 0xdead;
        }
      }
    }

    


    /*
    theApp.compute( &config, sizeof(config));
    theApp.join();
    */

    config.set_modeLoad( true);
    config.set_modeCompute( true);

#ifdef NUM_OF_THREADS
    unsigned int max_count = 20*NUM_OF_THREADS/5;
    if ( max_count > 40) {
      max_count = 40;
    }
#else    
    unsigned int max_count = 20*30;
#endif

    for ( unsigned int count=0; count<max_count; ++count) {
#ifdef USE_TBB
      tbb::tick_count tbb_begin = tbb::tick_count::now();
#endif
      for ( unsigned int icount=0; icount<40; ++icount) {
	theApp.compute( &config, sizeof(config));
	theApp.join();
      }
#ifdef USE_TBB
      tbb::tick_count tbb_end = tbb::tick_count::now();
      double tbb_wall_secs = (tbb_end-tbb_begin).seconds();
      std::cout << "count: " << count << " " << tbb_wall_secs << " seconds" << std::endl;
#endif
    }

    {

      unsigned int badCount = 0;


      // This seems to be a way to compute k
      auto computeK = [n]( unsigned int s, unsigned int t) {
          return t*n+s-(t+1)*(t+2)/2;
      };

      short res[nSlices][n];

      unsigned int k = 0;
      for( unsigned int t=0; t<n; ++t) {

        short weights[n];
        for ( unsigned int j=0; j<n; ++j) {
          weights[j] = wi_ptr[n*t+j];
        }

        unsigned int k0 = k;
        for( unsigned int s=t+1; s<n; ++s) {
          for ( unsigned int j=0; j<n; ++j) {
            weights[j] += mi_ptr[n*k+j];
          }
          assert( k == computeK( s, t));
          ++k;
        }
        // backward weight updates (get from res)
        for( unsigned int s=0; s<t; ++s) {
          // Computing k with s and t swapped
          unsigned int kp = computeK( t, s);

          for ( unsigned int j=0; j<n; ++j) {
            weights[j] += res[kp][j];
          }
        }

        for( unsigned int s=t+1; s<n; ++s) {
          unsigned int gamma = gi_ptr[s];

          short off[n];
          for ( unsigned int j=0; j<n; ++j) {
            off[j] = ((gamma*weights[j])>>10) - mi_ptr[n*k0+j];
          }

          short out[n];

          // Perform the loaf computation
          unsigned short b = lof_ptr[k0].a;
          unsigned short f = lof_ptr[k0].e;

          for ( unsigned int j=0; j<n; ++j) {
            short best = 0; // get compiler to shutup
            for ( unsigned int i=0; i<n; ++i) {            
              unsigned int idx = n*i+j;
              unsigned short a = slc_ptr[idx].a;
              unsigned short e = slc_ptr[idx].e;
              short o = off[i];

              unsigned short mx = (e>f)?e:f;
              unsigned short ab = (a>b)?a-b:b-a;
              const unsigned short threshold = 204;
              unsigned short mn = (ab<threshold)?ab:threshold;
              short cand = o + (((unsigned int) mx*(unsigned int) mn)>>10);

              if ( i == 0 || (cand < best)) {
                best = cand;
              }
            }
            out[j] = best;
          }

          // Perform clamping operation
          {
            short best = 0; // get compiler to shutup
            for ( unsigned int j=0; j<n; ++j) {
              short cand = out[j];
              if ( (j==0) || cand < best) {
                best = cand;
              }
            }
            for ( unsigned int j=0; j<n; ++j) {
              res[k0][j] = out[j] - best;
            }
          }
          ++k0;
        }
      }




      for( unsigned int k=0; k<nSlices; ++k) {
        for ( unsigned int j=0; j<n; ++j) {

          unsigned short gold = res[k][j];
          unsigned short cand = mo_ptr[n*k+j];

          if ( gold != cand) {
            ++badCount;
            std::cout << "MISMATCH: gold,mo,k,j: "
                      << gold << ","
                      << cand << ","
                      << k << ","
                      << j << std::endl;
          } else {
/*
            std::cout << "MATCH: gold,mo,k,j: "
                      << gold << ","
                      << cand << ","
                      << k << ","
                      << j << std::endl;
*/
          }
        }
      }

      EXPECT_EQ( badCount, 0U);

    }

    theApp.free();
  } else {
    EXPECT_TRUE( 0);
  }
}

int main (int argc, char *argv[]) {
#ifdef USE_TBB
#ifdef NUM_OF_THREADS
  std::cout << "Number of threads: " << NUM_OF_THREADS << std::endl;
  tbb::task_scheduler_init init(NUM_OF_THREADS);
#endif
#endif

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
