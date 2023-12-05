add_library(wmcv-memory "" )

target_sources(
    wmcv-memory PRIVATE
        pch.h
        wmcv_arena_allocator.cpp
        wmcv_stack_allocator.cpp
        wmcv_block_allocator.cpp
        wmcv_allocator_padding.h
        wmcv_allocator_padding.cpp
        wmcv_freelist_first_fit_policy.h
        wmcv_freelist_first_fit_policy.cpp
        wmcv_freelist_best_fit_policy.h
        wmcv_freelist_best_fit_policy.cpp
        wmcv_intrusive_red_black_tree.h
        wmcv_intrusive_red_black_tree.cpp
)

if( MSVC )
    target_sources(wmcv-memory PRIVATE pch.cpp)
endif()

target_precompile_headers(wmcv-memory PRIVATE pch.h pch.cpp)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

if( ENABLE_ALL_REASONABLE_WARNINGS )
    MESSAGE("-- Additional Warnings Enabled")
    target_enable_warnings(wmcv-memory)
endif()

if( ENABLE_WARNINGS_AS_ERRORS )
    MESSAGE("-- Warnings as Errors")
    target_warnings_as_errors(wmcv-memory)
endif()

if( ENABLE_SANITIZERS )
    MESSAGE("-- Sanitizers Enabled")
    target_enable_sanitizers(wmcv-memory)
endif()

if( ENABLE_STATIC_ANALYSIS )
    MESSAGE("-- Static Analysis Enabled")
    target_enable_static_analysis(wmcv-memory)
endif()