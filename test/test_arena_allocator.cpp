#include "test_pch.h"

#include "wmcv_memory/wmcv_arena_allocator.h"
#include "wmcv_memory/wmcv_allocator_utility.h"

TEST(test_arena_allocator, test_allocator_alloc)
{
	wmcv::Block mem{.address = 0x00040000, .size = 4_kB};
	wmcv::ArenaAllocator arena(mem);

	auto result = arena.allocate(1_kB);
	EXPECT_NE(result, wmcv::NullBlock());

	EXPECT_EQ(result.address, 0x00040000);
	EXPECT_EQ(result.size, 1_kB);
}

TEST(test_arena_allocator, test_allocator_alloc_aligned)
{
	wmcv::Block mem{.address = 0x00040007, .size = 4_kB};
	wmcv::ArenaAllocator arena(mem);

	constexpr size_t alignment = 32;
	constexpr size_t size = 64;

	auto result = arena.allocate_aligned(size, alignment);
	EXPECT_NE(result, wmcv::NullBlock());
	EXPECT_TRUE(wmcv::is_aligned(result.address, alignment));

	result = arena.allocate_aligned(size, alignment);
	EXPECT_NE(result, wmcv::NullBlock());
	EXPECT_TRUE(wmcv::is_aligned(result.address, alignment));
}

TEST(test_arena_allocator, test_allocator_alloc_too_large)
{
	wmcv::Block mem{.address = 0x00040000, .size = 4_kB};
	wmcv::ArenaAllocator arena(mem);

	constexpr size_t size = 8_kB;

	auto result = arena.allocate(size);
	EXPECT_EQ(result, wmcv::NullBlock());
}

TEST(test_arena_allocator, test_allocator_clear)
{
	wmcv::Block mem{.address = 0x00040000, .size = 4_kB};
	wmcv::ArenaAllocator arena(mem);

	constexpr size_t size = 3_kB;
	auto block = arena.allocate(size);
	EXPECT_NE(block, wmcv::NullBlock());

	block = arena.allocate(size);
	EXPECT_EQ(block, wmcv::NullBlock());

	arena.reset();

	block = arena.allocate(size);
	EXPECT_NE(block, wmcv::NullBlock());
}