#include "ulp.hpp"

#include <gtest/gtest.h>
#include <boost/multiprecision/mpfr.hpp>
#include <array>
#include <algorithm>
#include <ios>
#include <cmath>
#include <type_traits>

template<typename T>
void print_wrapper(const T &val) {
  std::cout << val << std::endl;
}

void print_mpfr_100(const boost::multiprecision::mpfr_float_100 &val) {
  print_wrapper(val);
}

void print_mpfr_500(const boost::multiprecision::mpfr_float_500 &val) {
  print_wrapper(val);
}

namespace calc_ulp {

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

template <typename PreciseFloat, typename BuiltinFloat, int size>
std::array<PreciseFloat, size> get_ref() {
  auto data = get_ref_data<BuiltinFloat>();
  static_assert(size <= data.size(), "more than available data is required");
  std::array<PreciseFloat, size> ref;
  std::copy_n(data.begin(), size, ref.begin());
  return ref;
}

template <typename PreciseFloat, typename BuiltinFloat> void test() {
  constexpr int num = 3;
  std::array<PreciseFloat, num> val = {
      PreciseFloat{"1.2429840317239821192837128310809123809211209821234245898"},
      PreciseFloat{"1.9999999999999999999999999999999999999999999999999999999"},
      0.0};
  auto ref = get_ref<PreciseFloat, BuiltinFloat, num>();
  std::array<PreciseFloat, num> res;
  std::transform(val.begin(), val.end(), res.begin(),
                 [](auto el) { return ulp::calc_ulp<BuiltinFloat>(el); });
  auto mism = std::mismatch(res.begin(), res.end(), ref.begin());
  ASSERT_EQ(mism.first, res.end()) << std::hexfloat << "input " <<
    val[mism.first - res.begin()] << " returned " << *mism.first <<
    " expected " << *mism.second << std::endl;
}

} // namespace calc_ulp

namespace analyze_func_ulp {

template <typename FloatT> std::array<FloatT, 7> get_arg_data();

template<>
std::array<float, 7> get_arg_data<float>() {
  return {0.0, 0.1, 1.0, 2.0, 42.0, 83.0, -83.5};
}

template<>
std::array<double, 7> get_arg_data<double>() {
  return {0.0, 0.1, 1.0, 2.0, 42.0, 705.0, -705.5};
}

template <typename BuiltinFloat, int size>
std::array<BuiltinFloat, size> get_arg() {
  auto data = get_arg_data<BuiltinFloat>();
  static_assert(size <= data.size(), "more than available data is required");
  std::array<BuiltinFloat, size> arg;
  std::copy_n(data.begin(), size, arg.begin());
  return arg;
}

template <typename BuiltinFunc, typename PreciseFunc,
          typename RestrictionFunc>
void test(BuiltinFunc builtin_func, PreciseFunc precise_func,
          RestrictionFunc restriction_func) {
  using BuiltinFloat = std::remove_reference_t<decltype(builtin_func(0.0))>;
  constexpr int num = 7;
  std::array<BuiltinFloat, num> arg = get_arg<BuiltinFloat, num>();
  std::array<BuiltinFloat, num> res;
  std::transform(arg.begin(), arg.end(), res.begin(), builtin_func);
  std::array<BuiltinFloat, num> error;
  ulp::analyze_func_ulp(arg.begin(), arg.end(), res.begin(),
                        error.begin(), precise_func);
  auto it = std::find_if_not(error.begin(), error.end(), restriction_func);
  auto idx = it - error.begin();
  ASSERT_EQ(it, error.end()) << "input: " << arg[idx] << " output: " <<
    res[idx] << "doesn't meet ULP error requirements with error " <<
    error[idx] << " ULP" << std::endl;
}

} // namespace analyze_func_ulp

TEST(boost, flt_100) {
  calc_ulp::test<boost::multiprecision::mpfr_float_100, float>();
}

TEST(boost, flt_500) {
  calc_ulp::test<boost::multiprecision::mpfr_float_500, float>();
}

TEST(boost, dbl_100) {
  calc_ulp::test<boost::multiprecision::mpfr_float_100, double>();
}

TEST(boost, dbl_500) {
  calc_ulp::test<boost::multiprecision::mpfr_float_500, double>();
}

TEST(boost, flt_std_exp_500) {
  analyze_func_ulp::test([] (float el) { return std::exp(el); },
    [] (boost::multiprecision::mpfr_float_500 el) -> boost::multiprecision::mpfr_float_500
      { return boost::math::expm1(el) + 1.0; },
    [] (auto el) { return std::abs(el) < 1.0; });
}

TEST(boost, dbl_std_exp_500) {
  analyze_func_ulp::test([] (double el) { return std::exp(el); },
    [] (boost::multiprecision::mpfr_float_500 el) -> boost::multiprecision::mpfr_float_500
      { return boost::math::expm1(el) + 1.0; },
    [] (auto el) { return std::abs(el) < 1.0; });
}

TEST(boost, flt_std_exp_dbl) {
  analyze_func_ulp::test([] (float el) { return std::exp(el); },
    [] (double el) { return std::exp(el); },
    [] (auto el) { return std::abs(el) < 1.0; });
}

TEST(boost, flt_std_exp_flt) {
  analyze_func_ulp::test([] (float el) { return std::exp(el); },
    [] (float el) { return std::exp(el); },
    [] (auto el) { return el == 0.0; });
}

TEST(boost, dbl_std_exp_dbl) {
  analyze_func_ulp::test([] (double el) { return std::exp(el); },
    [] (double el) { return std::exp(el); },
    [] (auto el) { return el == 0.0; });
}
