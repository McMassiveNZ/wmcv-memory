#ifndef WMCV_MEMORY_TEST_PCH_H_INCLUDED
#define WMCV_MEMORY_TEST_PCH_H_INCLUDED

#include <cinttypes>
#include <cassert>
#include <cstddef>
#include <cstring>

#include <type_traits>
#include <algorithm>
#include <numeric>
#include <memory>
#include <array>
#include <span>
#include <stack>
#include <utility>

#include <gtest/gtest.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <iostream>
#endif

#endif //WMCV_MEMORY_TEST_PCH_H_INCLUDED