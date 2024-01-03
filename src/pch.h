#ifndef WMCV_MEMORY_PCH_H_INCLUDED
#define WMCV_MEMORY_PCH_H_INCLUDED

#include <cinttypes>
#include <cassert>
#include <cstddef>
#include <cstring>

#include <type_traits>
#include <algorithm>
#include <span>
#include <vector>
#include <atomic>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#else
#include <iostream>
#endif

#endif //WMCV_MEMORY_PCH_H_INCLUDED