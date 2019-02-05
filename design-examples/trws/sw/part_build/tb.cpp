// See LICENSE for license details.
#include "gtest/gtest.h"
#include "hld_alloc.h"

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

TEST(AccelTest, SimpleTest) {

  std::default_random_engine generator;
  std::uniform_int_distribution<short> dist_short(-(1<<14),(1<<14)-1);
  std::uniform_int_distribution<unsigned short> dist_ushort(0,(1<<14)-1);

#if defined(__AAL_USER__)
  AcclApp theApp("somestupidAFUID");
#else
  AcclApp theApp;
#endif

  const unsigned int nCLperRow = 3;
  const unsigned int n = 32*nCLperRow;
  unsigned int nSlices = n*(n-1)/2;

  unsigned long long sz = 2ULL*(  n
                                + n*n
                                + 2*(n*n + nSlices)
                                + nSlices*n
                                + nSlices*n
                                + nSlices*n
                                + nSlices*n );

  std::cout << "sz: " << sz << std::endl;

  if ( theApp.alloc( sz)) {
    unsigned char *WORKSPACE = theApp.m_JointVirt;
    size_t WORKSPACE_SIZE    = theApp.m_JointSize;

    Config config;

    hld_alloc altor((char *)WORKSPACE, WORKSPACE_SIZE);

    UCacheLine *gi_ptr = altor.allocate<UCacheLine>( 1ULL*nCLperRow);
    CacheLine *wi_ptr = altor.allocate<CacheLine>( 1ULL*n*nCLperRow);
    Pair *inp_ptr = altor.allocate<Pair>( 1ULL*n*n + nSlices);

    UCacheLine *mi_ptr = altor.allocate<UCacheLine>( 1ULL*nSlices*nCLperRow);
    UCacheLine *mo_ptr = altor.allocate<UCacheLine>( 1ULL*nSlices*nCLperRow);

    config.set_aGi( (AddrType) gi_ptr);
    config.set_aWi( (AddrType) wi_ptr);
    config.set_aInp( (AddrType) inp_ptr);
    config.set_aMi( (AddrType) mi_ptr);
    config.set_aMo( (AddrType) mo_ptr);

    config.set_nCLperRow( nCLperRow);
    config.set_nSlices( nSlices);

    {
      for ( unsigned int jj=0; jj<nCLperRow; ++jj) {
          for ( unsigned int j=0; j<32; ++j) {
              gi_ptr[jj].words[j] = dist_ushort( generator) % 8;
          }
      }
    }
    {
      for ( unsigned int i=0; i<n; ++i) {
          for ( unsigned int jj=0; jj<nCLperRow; ++jj) {
              for ( unsigned int j=0; j<32; ++j) {
                  wi_ptr[nCLperRow*i+jj].words[j] = dist_short( generator);
              }
          }
      }
    }

    {
      for ( unsigned int i=0; i<n; ++i) {
        for ( unsigned int j=0; j<n; ++j) {
          inp_ptr[n*i+j].a = dist_short( generator);
          inp_ptr[n*i+j].e = dist_ushort( generator);
        }
      }
      for ( unsigned int k=0; k<nSlices; ++k) {
          inp_ptr[n*n+k].a = dist_short( generator);
          inp_ptr[n*n+k].e = dist_ushort( generator);
      }
    }

    {
      for( unsigned int k=0; k<nSlices; ++k) {
        for ( unsigned int jj=0; jj<nCLperRow; ++jj) {
          for ( unsigned int j=0; j<32; ++j) {
            mi_ptr[nCLperRow*k+jj].words[j] = dist_ushort( generator);
            mo_ptr[nCLperRow*k+jj].words[j] = 0xdead;
          }
        }
      }
    }

    theApp.compute( &config, sizeof(config));
    theApp.join();

    {
      unsigned int badCount = 0;

      CacheLine weights[n][nCLperRow];
      for( unsigned int i=0; i<n; ++i) {
        for ( unsigned int jj=0; jj<nCLperRow; ++jj) {
          for ( unsigned int j=0; j<32; ++j) {
            weights[i][jj].words[j] = wi_ptr[nCLperRow*i+jj].words[j];
          }
        }
      }

      // This seems to be a way to compute k
      auto computeK = [n]( unsigned int s, unsigned int t) {
          return t*n+s-(t+1)*(t+2)/2;
      };

      CacheLine res[nSlices][nCLperRow];

      unsigned int k = 0;
      for( unsigned int t=0; t<n; ++t) {
        unsigned int k0 = k;
        for( unsigned int s=t+1; s<n; ++s) {
          for ( unsigned int jj=0; jj<nCLperRow; ++jj) {
            for ( unsigned int j=0; j<32; ++j) {
              weights[t][jj].words[j] += mi_ptr[nCLperRow*k+jj].words[j];
            }
          }
          assert( k == computeK( s, t));
          ++k;
        }
        // backward weight updates (get from res)
        for( unsigned int s=0; s<t; ++s) {
          // Computing k with s and t swapped
          unsigned int kp = computeK( t, s);

          for ( unsigned int jj=0; jj<nCLperRow; ++jj) {
            for ( unsigned int j=0; j<32; ++j) {
              weights[t][jj].words[j] += res[kp][jj].words[j];
            }
          }
        }
        for( unsigned int s=t+1; s<n; ++s) {
          unsigned int gamma = gi_ptr[s/32].words[s%32];
          CacheLine off[nCLperRow];
          for ( unsigned int jj=0; jj<nCLperRow; ++jj) {
            for ( unsigned int j=0; j<32; ++j) {
              off[jj].words[j] = gamma*weights[t][jj].words[j] - mi_ptr[nCLperRow*k0+jj].words[j];
            }
          }
          CacheLine out[nCLperRow];

          // Perform the loaf computation
          short b = inp_ptr[n*n+k0].a;
          unsigned short f = inp_ptr[n*n+k0].e;

          for ( unsigned int jj=0; jj<nCLperRow; ++jj) {
            for ( unsigned int j=0; j<32; ++j) {
              short best = 0; // get compiler to shutup
              for ( unsigned int i=0; i<n; ++i) {            
                short a = inp_ptr[n*i+jj*32+j].a;
                unsigned short e = inp_ptr[n*i+jj*32+j].e;
                short o = off[i/32].words[i%32];

                unsigned short mx = (e>f)?e:f;
                unsigned short ab = (a>b)?a-b:b-a;
                const unsigned short threshold = 204;
                unsigned short mn = (ab<threshold)?ab:threshold;
                short cand = o + (((unsigned int) mx*(unsigned int) mn)>>16);

                if ( i == 0 || (cand < best)) {
                  best = cand;
                }
              }
              out[jj].words[j] = best;
            }
          }

          // Perform clamping operation
          {
            short best = 0; // get compiler to shutup
            for ( unsigned int jj=0; jj<nCLperRow; ++jj) {
              for ( unsigned int j=0; j<32; ++j) {
                short cand = out[jj].words[j];
                if ( (jj==0 && j==0) || cand < best) {
                  best = cand;
                }
              }
            }
            //            best = 0;
            for ( unsigned int jj=0; jj<nCLperRow; ++jj) {
              for ( unsigned int j=0; j<32; ++j) {
                res[k0][jj].words[j] = out[jj].words[j] - best;
              }
            }
          }
          ++k0;
        }
      }

      for( unsigned int k=0; k<nSlices; ++k) {
        for ( unsigned int jj=0; jj<nCLperRow; ++jj) {
          for ( unsigned int j=0; j<32; ++j) {

            unsigned short gold = res[k][jj].words[j];
            unsigned short cand = mo_ptr[nCLperRow*k+jj].words[j];

            if ( gold != cand) {
              ++badCount;
              std::cout << "MISMATCH: gold,mo,k,jj,j: "
                        << gold << ","
                        << cand << ","
                        << k << ","
                        << jj << ","
                        << j << std::endl;
            } else {
/*
              std::cout << "MATCH: gold,mo,k,jj,j: "
                        << gold << ","
                        << cand << ","
                        << k << ","
                        << jj << ","
                        << j << std::endl;
*/
            }
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
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
