#include "ulp.hpp"

#include <gtest/gtest.h>
#include <array>
#include <algorithm>
#include <ios>

TEST(basic, flt) {
  constexpr int num = 5;
  std::array<double, num> val = {1.3,     0.0,      1.0,     -5.0,    -129.0};
  std::array<double, num> ref = {0x1p-23, 0x1p-149, 0x1p-23, 0x1p-21, 0x1p-16};
  std::array<double, num> res;
  std::transform(val.begin(), val.end(), res.begin(),
    [](auto el) { return ulp::calc_ulp<float>(el); });
  auto mism = std::mismatch(res.begin(), res.end(), ref.begin());
  ASSERT_EQ(mism.first, res.end()) << std::hexfloat << "input " <<
    val[mism.first - res.begin()] << " returned " << *mism.first <<
    " expected " << *mism.second << std::endl;
}

TEST(basic, dbl) {
  constexpr int num = 5;
  std::array<double, num> val = {1.3,     0.0,       1.0,     -5.0,    -129.0};
  std::array<double, num> ref = {0x1p-52, 0x1p-1074, 0x1p-52, 0x1p-50, 0x1p-45};
  std::array<double, num> res;
  std::transform(val.begin(), val.end(), res.begin(),
    [](auto el) { return ulp::calc_ulp<double>(el); });
  auto mism = std::mismatch(res.begin(), res.end(), ref.begin());
  ASSERT_EQ(mism.first, res.end()) << std::hexfloat << "input " <<
    val[mism.first - res.begin()] << " returned " << *mism.first <<
    " expected " << *mism.second << std::endl;
}
