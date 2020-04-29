#include "ulp.hpp"

#include <gtest/gtest.h>
#include <boost/multiprecision/mpfr.hpp>
#include <array>
#include <algorithm>
#include <ios>

template<typename FloatT>
std::array<FloatT, 3> get_ref_data();

template<>
std::array<double, 3> get_ref_data<double>() {
  return {
    0x1p-52,
    0x1p-52,
    0x1p-1074
  };
}

template<>
std::array<float, 3> get_ref_data<float>() {
  return {
    0x1p-23,
    0x1p-23,
    0x1p-149
  };
}

template<typename TestFloat, typename ReprFloat, int size>
std::array<TestFloat, size> get_ref() {
  auto data = get_ref_data<ReprFloat>();
  static_assert(size <= data.size(), "more than available data is required");
  std::array<TestFloat, size> ref;
  std::copy_n(data.begin(), size, ref.begin());
  return ref;
}

template<typename TestFloat, typename ReprFloat>
void test() {
  constexpr int num = 3;
  std::array<TestFloat, num> val = {
    TestFloat{"1.2429840317239821192837128310809123809211209821234245898"},
    TestFloat{"1.9999999999999999999999999999999999999999999999999999999"},
    0.0
  };
  auto ref = get_ref<TestFloat, ReprFloat, num>();
  std::array<TestFloat, num> res;
  std::transform(val.begin(), val.end(), res.begin(),
    [](auto el) { return ulp::calc_ulp<ReprFloat>(el); });
  auto mism = std::mismatch(res.begin(), res.end(), ref.begin());
  ASSERT_EQ(mism.first, res.end()) << std::hexfloat << "input " <<
    val[mism.first - res.begin()] << " returned " << *mism.first <<
    " expected " << *mism.second << std::endl;
}

TEST(boost, flt_100) {
  test<boost::multiprecision::mpfr_float_100, float>();
}

TEST(boost, flt_500) {
  test<boost::multiprecision::mpfr_float_500, float>();
}

TEST(boost, dbl_100) {
  test<boost::multiprecision::mpfr_float_100, double>();
}

TEST(boost, dbl_500) {
  test<boost::multiprecision::mpfr_float_500, double>();
}
