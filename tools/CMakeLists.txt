cmake_minimum_required(VERSION 3.0)

set(FERROR_BIN ferror)

add_executable(${FERROR_BIN} func_error.cpp)
target_compile_features(${FERROR_BIN} PUBLIC cxx_std_17)
target_link_libraries(${FERROR_BIN} PUBLIC ulplib)
target_link_libraries(${FERROR_BIN} PUBLIC Boost::headers)
target_link_libraries(${FERROR_BIN} PUBLIC mpfr)
