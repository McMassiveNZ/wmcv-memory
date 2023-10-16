target_sources(
    wmcv-memory
        PRIVATE
            wmcv_memory/wmcv_arena_allocator.h
            wmcv_memory/wmcv_stack_allocator.h
            wmcv_memory/wmcv_pool_allocator.h
            wmcv_memory/wmcv_allocator_utility.h
            wmcv_memory/wmcv_memory_block.h
)

target_include_directories(
    wmcv-memory
        PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}/wmcv_memory
        PUBLIC
            ${CMAKE_CURRENT_LIST_DIR}
)