#include <ios>
#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>
#include <functional>
#include <cassert>
#include <cstdlib>

#include <boost/multiprecision/mpfr.hpp>
#include "ulp.hpp"

// In some versions of libstd++ std::hexfloat qualifier
// doesn't work in input stream.
// TODO: check and set this define in cmake
#define FAULTY_LIBSTDCPP

// it's meant to be quick and dirty
// TODO: someday make a good testing app out of it

namespace {

class command_args_info {
public:
  enum class function {
    exp,
    log
  };

  enum class fptype {
    flt,
    dbl
  };

private:
  std::string input_file_{};
  function func_;
  fptype type_;
  bool is_error_{false};

public:
  command_args_info(const char *input_file = "", function func = function::exp,
                    fptype type = fptype::flt) :
    input_file_{input_file}, func_{func}, type_{type}, is_error_{false} {}

  static command_args_info create_error() {
    command_args_info err;
    err.is_error_ = true;
    return err;
  }

  bool is_error() const {
    return is_error_;
  }

  const std::string &get_input_file() const {
    check();
    return input_file_;
  }

  function get_function() const {
    check();
    return func_;
  }

  fptype get_type() const {
    check();
    return type_;
  }

private:
  void check() const {
    assert(!is_error() && "args are invalid");
  }
};

command_args_info::function parse_function_name(const char *str) {
  if (str == std::string_view{"exp"}) {
    return command_args_info::function::exp;
  } else if (str == std::string_view{"log"}) {
    return command_args_info::function::log;
  } else {
    std::cout << "Unsupported function" << std::endl;
    exit(0);
  }
}

command_args_info::fptype parse_fptype(const char *str) {
  if (str == std::string_view{"float"}) {
    return command_args_info::fptype::flt;
  } else if (str == std::string_view{"double"}) {
    return command_args_info::fptype::dbl;
  } else {
    std::cout << "Unsupported type" << std::endl;
    exit(0);
  }
}

command_args_info parse_args(int argc, const char * const *argv) {
  if (argc < 2) {
    std::cout << "Please enter the floating-point type, "
      "function name and path to input file" << std::endl;
    return command_args_info::create_error();
  }
  if (argc < 3) {
    std::cout << "Please enter the function name and path to input file"
      << std::endl;
    return command_args_info::create_error();
  }
  if (argc < 4) {
    std::cout << "Please enter the path to input file"
      << std::endl;
    return command_args_info::create_error();
  }
  if (argc > 4) {
    std::cout << "Too much arguments" << std::endl;
    return command_args_info::create_error();
  }
  return {argv[3], parse_function_name(argv[2]), parse_fptype(argv[1])};
}

template<typename FloatT>
struct func_points {
  std::vector<FloatT> arguments_;
  std::vector<FloatT> values_;

  func_points(int size) {
    reserve(size);
  }

  int size() const {
    check();
    return arguments_.size();
  }

  void reserve(int size) {
    check();
    arguments_.reserve(size);
    values_.reserve(size);
  }

  void push_back(FloatT argument, FloatT value) {
    check();
    arguments_.push_back(argument);
    values_.push_back(value);
  }

  void dump() const {
    check();
    for (int i = 0; i < arguments_.size(); ++i) {
      std::cout << std::hexfloat << arguments_[i] <<
        " " << values_[i] << std::endl;
    }
  }

private:
  void check() const {
    assert(arguments_.size() == values_.size() &&
      "number of arguments and values are inconsistent");
  }
};

template<typename FloatT>
func_points<FloatT> read_input_file(const std::string &file_name) {
  std::ifstream file{file_name};
  if (!file.is_open()) {
    std::cout << "Cannot open the provided file" << std::endl;
    std::exit(0);
  }
  int data_size;
  file >> data_size;
  func_points<FloatT> res{data_size};
  for (int i = 0; i < data_size; ++i) {
    FloatT argument;
    FloatT value;
#ifdef FAULTY_LIBSTDCPP
    std::string line;
    char *next_float;
    file >> line;
    argument = std::strtod(line.c_str(), &next_float);
    file >> line;
    value = std::strtod(line.c_str(), &next_float);
#else
    file >> std::hexfloat >> argument >> value >> std::defaultfloat;
#endif
    res.push_back(argument, value);
  }
  return res;
}

std::function<boost::multiprecision::mpfr_float_500(boost::multiprecision::mpfr_float_500)>
get_function(command_args_info::function func_id) {
  switch(func_id) {
  case command_args_info::function::exp:
    return [] (boost::multiprecision::mpfr_float_500 el) -> boost::multiprecision::mpfr_float_500
      { return boost::math::expm1(el) + 1.0; };
  case command_args_info::function::log:
    return [] (boost::multiprecision::mpfr_float_500 el) -> boost::multiprecision::mpfr_float_500
      { return boost::math::log1p(el - 1.0); };
  default:
    assert(0 && "unsupported function");
  }
}

} // anonymous namespace

template<typename FloatT, typename Func>
func_points<FloatT> analyze_func_error(const func_points<FloatT> &input_data, Func func) {
  func_points<FloatT> error(input_data.size());
  std::copy(input_data.arguments_.begin(), input_data.arguments_.end(),
    std::back_inserter(error.arguments_));
  ulp::analyze_func_ulp(input_data.arguments_.begin(), input_data.arguments_.end(),
    input_data.values_.begin(), std::back_inserter(error.values_), func);
  return error;
}

template<typename FloatT>
void run_job(const command_args_info &args) {
  auto input_data = read_input_file<FloatT>(args.get_input_file());
  auto func = get_function(args.get_function());
  auto error_data = analyze_func_error(input_data, func);
  error_data.dump();
}

int main(int argc, const char * const *argv) {
  auto args = parse_args(argc, argv);
  if (args.is_error()) {
    return 0;
  }
  switch(args.get_type()) {
  case command_args_info::fptype::flt:
    run_job<float>(args);
    break;
  case command_args_info::fptype::dbl:
    run_job<double>(args);
    break;
  }
  return 0;
}
