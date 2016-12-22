// See LICENSE for license details.
#include "gtest/gtest.h"
#include <iostream>

#include <string.h>

TEST(AccelTest, SimpleTest) {

  unsigned int n = 16*1024;

  unsigned int *ina_ptr = (unsigned int *) malloc(sizeof(unsigned int)*n);
  unsigned int *inb_ptr = (unsigned int *) malloc(sizeof(unsigned int)*n);
  unsigned int *out_ptr = (unsigned int *) malloc(sizeof(unsigned int)*n);

  for( unsigned int ip=0; ip<n; ++ip) {
    ina_ptr[ip] = ip;
    inb_ptr[ip] = n-ip;
  }

  for( unsigned int ip=0; ip<n; ++ip) {
    out_ptr[ip] = 0xdeadbeefU;
  }

  for( unsigned int ip=0; ip<n; ++ip) {
    out_ptr[ip] = ina_ptr[ip] + inb_ptr[ip];
  }

  // check
  unsigned int correct = 0;
  for (unsigned int ip=0; ip<n; ++ip) {
    unsigned int cand = out_ptr[ip];
    unsigned int ref  = n;
    if ( cand == 0xdeadbeefU) {
      std::cout << "Uninitialized result at " << ip << std::endl;
    }

    if ( cand == ref) {
      ++correct;
    } else {
      std::cout << ip << " != " << ref << " " << cand << std::endl;
    }
  }

  std::cout << "Results checked: " << correct
            << " of " << n << " correct." << std::endl;

  EXPECT_EQ( correct, n);

  free( ina_ptr);
  free( inb_ptr);
  free( out_ptr);
}

int main (int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
