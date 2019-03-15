
#include "gtest/gtest.h"

#include "types.h"
#include "hls_utils.h"
#include <iostream>

TEST(CommonTest, Log2UpperTest)
{
  EXPECT_EQ( Log2Upper<9>::Value, 4);
  EXPECT_EQ( Log2Upper<8>::Value, 3);
  EXPECT_EQ( Log2Upper<7>::Value, 3);
  EXPECT_EQ( Log2Upper<6>::Value, 3);
  EXPECT_EQ( Log2Upper<5>::Value, 3);
  EXPECT_EQ( Log2Upper<4>::Value, 2);
  EXPECT_EQ( Log2Upper<3>::Value, 2);
  EXPECT_EQ( Log2Upper<2>::Value, 1);
  EXPECT_EQ( Log2Upper<1>::Value, 0);
}

TEST(CommonTest, Log2Test)
{
  EXPECT_EQ( Log2<8>::Value, 3);
  EXPECT_EQ( Log2<4>::Value, 2);
  EXPECT_EQ( Log2<2>::Value, 1);
  EXPECT_EQ( Log2<1>::Value, 0);
}

int main (int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
