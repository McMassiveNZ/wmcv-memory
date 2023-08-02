include(gtest)

add_executable(
  wmcv-memory-test
  test_main.cpp
)

target_link_libraries(
  wmcv-memory-test
  gtest_main
)

include(GoogleTest)
gtest_discover_tests(wmcv-memory-test)
