// See LICENSE for license details.
//#include <stdio.h>
//#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "gtest/gtest.h"

#include "hld_alloc.h"

#if defined(USE_SOFTWARE) || defined(FPGA_ACC)
typedef unsigned long long UInt64;
#include "Config.h"
#include "AcclApp.h"
#else
#ifdef KERNEL_TEST
#include "linkedlist_hls_tb.h"
#else
#include "linkedlist_acc_tb.h"
#endif
#endif
 
#include <random>

TEST(AccelTest, SimpleTest) {

  unsigned long long m = 8*256;

  unsigned long long n = 1ULL << 16;

  std::default_random_engine generator;
  std::uniform_int_distribution<unsigned long long> distribution( 0, 2*n-1);

#if defined(FPGA_ACC)
  AcclApp theApp("deadbeef");
#else
  AcclApp theApp;
#endif

  size_t sz = 2*n*sizeof(Node)+2*m*sizeof(HeadPtr);

  if ( theApp.alloc( sz)) {
    unsigned char *WORKSPACE = theApp.m_JointVirt;
    size_t WORKSPACE_SIZE = theApp.m_JointSize;

    Config config;
  
    hld_alloc alloc((char*)WORKSPACE, WORKSPACE_SIZE);

    Node *nodes =  alloc.allocate<Node>(2*n);
    HeadPtr *heads = alloc.allocate<HeadPtr>(m);
    HeadPtr *results = alloc.allocate<HeadPtr>(m);

    //    config.set_aLst( (AddrType) nodes);
    config.set_aInp( (AddrType) heads);
    config.set_aOut( (AddrType) results);
    config.set_m( m);
  
    memset(nodes, 0, 2*n*sizeof(Node));

    // generate linked list
    unsigned int logG = 6;
    unsigned long long G = 1ULL<<logG;
    unsigned long long maskG = G-1;

    for( unsigned long long i=0; i<2*n; ++i) {
      nodes[i].val = i;
      if( i < n) {
        if ( (i+1) & maskG) {
          //          std::cout << i << " -> " << i+1 << std::endl;
          nodes[i].set_next( &nodes[i+1]);
        } else {
          //          std::cout << i << " -> " << (i & ~maskG) << std::endl;
          nodes[i].set_next( &nodes[i & ~maskG]);
        }
      } else {
        if ( (i+1) & maskG) {
          //          std::cout << i << " -> " << i+1 << std::endl;
          nodes[i].set_next( &nodes[i+1]);
        } else {
          //          std::cout << i << " -> " << "NULL" << std::endl;
          nodes[i].set_next( NULL);
        }
      }
    }

    for ( unsigned int ip = 0; ip < m; ++ip) {
      unsigned long long i = distribution( generator);
      //      std::cout << "ip,i:" << ip << "," << i << std::endl;
      heads[ip].set_head( nodes+i);
    }    

    theApp.compute(&config, sizeof(Config));
    theApp.join();

    unsigned long long correct = 0;

    for ( unsigned int ip = 0; ip < m; ++ip) {
      HeadPtr res = results[ip];
      bool hascycle = res.get_found();
      res.set_found( 0);

      bool shouldhavecycle = (unsigned long long)(res.get_head()-nodes)<n;

      if ( hascycle == shouldhavecycle) {
        ++correct;
      }
    }    
    std::cout << "Cycle determination correct " << correct << " out of " << m << " times." << std::endl;
    EXPECT_EQ( correct, m);
  }
  theApp.free();
}

int main (int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
