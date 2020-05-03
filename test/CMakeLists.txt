cmake_minimum_required(VERSION 3.10)

set(TESTBIN ulptest)

add_executable(${TESTBIN} basic_tests.cpp
                          boost_tests.cpp)
target_link_libraries(${TESTBIN} PUBLIC ulplib)
target_link_libraries(${TESTBIN} PUBLIC GTest::Main)
target_link_libraries(${TESTBIN} PUBLIC Boost::headers)
target_link_libraries(${TESTBIN} PUBLIC mpfr)
target_link_libraries(${TESTBIN} PUBLIC m)
