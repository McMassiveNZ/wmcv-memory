#include "test_pch.h"

#include "wmcv_memory/wmcv_lockless_arena_allocator.h"
#include "wmcv_memory/wmcv_allocator_utility.h"

TEST(test_lockless_arena_allocator, test_allocator_alloc)
{
    wmcv::Block mem{.address = 0x00040000, .size = 4_kB};
    wmcv::LocklessArenaAllocator arena(mem);
    
    auto result = arena.allocate(1_kB);
    EXPECT_NE(result, wmcv::NullBlock());
    
    EXPECT_EQ(result.address, 0x00040000);
    EXPECT_EQ(result.size, 1_kB);
}

TEST(test_lockless_arena_allocator, test_allocator_alloc_from_multiple_threads)
{
    wmcv::Block mem{.address = 0x00040000, .size = 4_kB};
    wmcv::LocklessArenaAllocator arena(mem);
    
    const size_t num_blocks = 8;
    const size_t num_threads = std::thread::hardware_concurrency();
    const size_t alloc_size = (mem.size / num_blocks);
    
    std::vector<std::thread> threads;
    std::vector<wmcv::Block> blocks(num_blocks);
    std::atomic_size_t num_threads_started = 0;
    std::atomic_size_t current_block_idx = 0;
    
    for (size_t i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([&]
                             {
            num_threads_started.fetch_add(1);
            while (num_threads_started.load() < num_threads)
                ;
            
            size_t idx = 0;
            do
            {
                if( idx = current_block_idx.fetch_add(1); idx < blocks.size() )
                {
                    blocks[idx] = arena.allocate(alloc_size);
                }
            }
            while(idx < blocks.size());
        });
    }
    
    for (auto& t : threads)
        t.join();
    
    std::sort(blocks.begin(), blocks.end());
    
    for ( size_t i = 0; i < blocks.size() - 1; ++i )
    {
        const auto B0 = blocks[i + 0];
        const auto B1 = blocks[i + 1];
        
        EXPECT_NE(B0, wmcv::NullBlock());
        
        const uintptr_t next = B1.address;
        const uintptr_t prev = B0.address + B0.size;
        EXPECT_LE(prev, next);
    }
}

TEST(test_lockless_arena_allocator, test_allocator_alloc_aligned)
{
    wmcv::Block mem{.address = 0x00040007, .size = 4_kB};
    wmcv::LocklessArenaAllocator arena(mem);
    
    constexpr size_t alignment = 32;
    constexpr size_t size = 64;
    
    auto result = arena.allocate_aligned(size, alignment);
    EXPECT_NE(result, wmcv::NullBlock());
    EXPECT_TRUE(wmcv::is_aligned(result.address, alignment));
    
    result = arena.allocate_aligned(size, alignment);
    EXPECT_NE(result, wmcv::NullBlock());
    EXPECT_TRUE(wmcv::is_aligned(result.address, alignment));
}

TEST(test_lockless_arena_allocator, test_allocator_alloc_too_large)
{
    wmcv::Block mem{.address = 0x00040000, .size = 4_kB};
    wmcv::LocklessArenaAllocator arena(mem);
    
    constexpr size_t size = 8_kB;
    
    auto result = arena.allocate(size);
    EXPECT_EQ(result, wmcv::NullBlock());
}

TEST(test_lockless_arena_allocator, test_allocator_clear)
{
    wmcv::Block mem{.address = 0x00040000, .size = 4_kB};
    wmcv::LocklessArenaAllocator arena(mem);
    
    constexpr size_t size = 3_kB;
    auto block = arena.allocate(size);
    EXPECT_NE(block, wmcv::NullBlock());
    
    block = arena.allocate(size);
    EXPECT_EQ(block, wmcv::NullBlock());
    
    arena.reset();
    
    block = arena.allocate(size);
    EXPECT_NE(block, wmcv::NullBlock());
}
