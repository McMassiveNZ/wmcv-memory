include(gtest)

add_executable(wmcv-memory-test "")

target_sources(
  wmcv-memory-test 
    PRIVATE
      test_pch.h
      test_arena_allocator.cpp
      test_stack_allocator.cpp
      test_pool_allocator.cpp
      test_allocator_utility.cpp
)

if(MSVC)
  target_sources(wmcv-memory-test PRIVATE test_pch.cpp)
endif()

target_link_libraries(
  wmcv-memory-test
  gtest_main
  wmcv-memory
)

target_precompile_headers(wmcv-memory-test PRIVATE test_pch.h test_pch.cpp)

include(GoogleTest)
gtest_discover_tests(wmcv-memory-test)
