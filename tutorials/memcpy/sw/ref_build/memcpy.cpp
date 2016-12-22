// See LICENSE for license details.
#include "gtest/gtest.h"
#include <iostream>

#include <string.h>

TEST(AccelTest, SimpleTest) {

  unsigned int n_ulls = 64*1024;

  unsigned long long *inp_ptr = (unsigned long long *) malloc(8*n_ulls);
  unsigned long long *out_ptr = (unsigned long long *) malloc(8*n_ulls);

  for( unsigned int ip=0; ip<n_ulls; ++ip) {
    inp_ptr[ip] = ip;
  }

  for( unsigned int ip=0; ip<n_ulls; ++ip) {
    out_ptr[ip] = 0xdeadbeefdeadbeefULL;
  }

  memcpy( out_ptr, inp_ptr, 8*n_ulls);

  // check
  unsigned int correct = 0;
  for (unsigned int ip=0; ip<n_ulls; ++ip) {
    unsigned long long cand = out_ptr[ip];
    unsigned long long ref  = ip;
    if ( cand == 0xdeadbeefdeadbeefULL) {
      std::cout << "Uninitialized result at " << ip << std::endl;
    }

    if ( cand == ref) {
      ++correct;
    } else {
      std::cout << ip << " != " << ref << " " << cand << std::endl;
    }
  }

  std::cout << "Results checked: " << correct
            << " of " << n_ulls << " correct." << std::endl;

  EXPECT_EQ( correct, n_ulls);

  free( inp_ptr);
  free( out_ptr);
}

int main (int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
