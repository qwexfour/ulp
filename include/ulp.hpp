#include <iterator>
#include <type_traits>
#include <limits>
#include <cmath>
#include <cassert>

namespace ulp {

namespace detail {

template<typename, typename, typename = void>
struct has_convert_to : std::false_type {};

template<typename T, typename To>
struct has_convert_to<T, To,
  std::void_t<decltype(std::declval<T>().template convert_to<To>())>> :
    std::true_type {};

template<typename To, typename From,
         typename Enable = void>
To float_cast_impl(const From &val) {
  if constexpr (has_convert_to<From, To>::value)
    return val.template convert_to<To>();
  else
    return static_cast<To>(val);
}

} // namespace detail

template<typename To, typename From>
To float_cast(const From &val) {
  return detail::float_cast_impl<To>(val);
}

template<typename RepresentFloatT, typename ValueFloatT>
ValueFloatT calc_ulp(const ValueFloatT &value) {
  static_assert(std::is_floating_point_v<RepresentFloatT>,
    "floating point type is expected as representation type");
  RepresentFloatT casted_value = float_cast<RepresentFloatT>(value);
  RepresentFloatT casted_lo;
  RepresentFloatT casted_hi;
  if (casted_value <= value) {
    casted_lo = casted_value;
    casted_hi = std::nextafter(casted_value,
      std::numeric_limits<RepresentFloatT>::infinity());
  } else {
    casted_hi = casted_value;
    casted_lo = std::nextafter(casted_value,
      -std::numeric_limits<RepresentFloatT>::infinity());
  }
  assert(casted_lo <= value && value <= casted_hi &&
    "value must belong range [casted_lo, casted_hi]");
  ValueFloatT lo = casted_lo;
  ValueFloatT hi = casted_hi;
  return casted_hi - casted_lo;
}

template<typename ArgIter, typename ResIter,
         typename ULPOutIter, typename Func>
void analyze_func_ulp(ArgIter first_arg, ArgIter last_arg,
                      ResIter first_res, ULPOutIter d_out_ulp,
                      Func ref_func) {
  using ResT = typename std::iterator_traits<ResIter>::value_type;
  for (; first_arg != last_arg; ++first_arg, ++first_res) {
    auto ref_res = ref_func(*first_arg);
    auto ulp = calc_ulp<ResT>(ref_res);
    decltype(ref_res) casted_res = *first_res;
    decltype(ref_res) error_in_ulps = (casted_res - ref_res) / ulp;
    *d_out_ulp++ = float_cast<ResT>(error_in_ulps);
  }
}

} // namespace ulp


