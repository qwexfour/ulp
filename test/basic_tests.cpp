#include "ulp.hpp"

#include <gtest/gtest.h>

TEST(basic, dumb_simple) {
  double val = 1.3;
  double ulp = ulp::calc_ulp<float>(val);
  ASSERT_EQ(ulp, 0x1p-23);
}
