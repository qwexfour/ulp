#include <ios>
#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>
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

class command_args_info {
  std::string input_file_{};
  bool is_error_{false};

public:
  command_args_info(const char *input_file = "") :
    input_file_{input_file}, is_error_{false} {}

  static command_args_info create_error() {
    command_args_info err;
    err.is_error_ = true;
    return err;
  }

  bool is_error() const {
    return is_error_;
  }

  const std::string &get_input_file() const {
    assert(!is_error() && "args are invalid");
    return input_file_;
  }
};

command_args_info parse_args(int argc, const char * const *argv) {
  if (argc < 2) {
    std::cout << "Please enter the path to input file" << std::endl;
    return command_args_info::create_error();
  }
  if (argc > 2) {
    std::cout << "Too much arguments" << std::endl;
    return command_args_info::create_error();
  }
  return {argv[1]};
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
      std::cout << std::hexfloat << "arg: " << arguments_[i] <<
        " val: " << values_[i] << std::endl;
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

template<typename FloatT>
func_points<FloatT> analyze_func_error(const func_points<FloatT> &input_data) {
  func_points<FloatT> error(input_data.size());
  std::copy(input_data.arguments_.begin(), input_data.arguments_.end(),
    std::back_inserter(error.arguments_));
  ulp::analyze_func_ulp(input_data.arguments_.begin(), input_data.arguments_.end(),
    input_data.values_.begin(), std::back_inserter(error.values_),
    [] (boost::multiprecision::mpfr_float_500 el) -> boost::multiprecision::mpfr_float_500
      { return boost::math::expm1(el) + 1.0; });
  return error;
}

int main(int argc, const char * const *argv) {
  auto args = parse_args(argc, argv);
  if (args.is_error()) {
    return 0;
  }
  auto input_data = read_input_file<double>(args.get_input_file());
  auto error_data = analyze_func_error(input_data);
  error_data.dump();
  return 0;
}
