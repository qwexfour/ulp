#include <type_traits>
#include <limits>
#include <cmath>
#include <cassert>

namespace ulp {

template<typename RepresentFloatT, typename ValueFloatT>
ValueFloatT calc_ulp(const ValueFloatT &value) {
  static_assert(std::is_floating_point_v<RepresentFloatT>,
    "floating point type is expected as representaion type");
  RepresentFloatT casted_value = value;
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

} // namespace ulp


