#include "test_pch.h"

#include "wmcv_memory/wmcv_arena_allocator.h"
#include "wmcv_memory/wmcv_allocator_utility.h"

TEST(test_arena_allocator, test_construct_allocator)
{
	std::array<std::byte, 4_kB> buffer = {};
	wmcv::ArenaAllocator arena(buffer.data(), buffer.size());

	EXPECT_EQ(arena.internal_get_marker(), 0);
	EXPECT_EQ(arena.internal_get_size(), 4_kB);
}

TEST(test_arena_allocator, test_allocator_alloc)
{
	std::array<std::byte, 4_kB> buffer = {};
	wmcv::ArenaAllocator arena(buffer.data(), buffer.size());

	void* ptr = arena.allocate(1_kB);
	EXPECT_NE(ptr, nullptr);

	EXPECT_EQ(arena.internal_get_size(), 4_kB);
}

TEST(test_arena_allocator, test_allocator_alloc_aligned)
{
	std::array<std::byte, 4_kB> buffer = {};
	wmcv::ArenaAllocator arena(buffer.data(), buffer.size());

	constexpr size_t alignment = 32;
	constexpr size_t size = 64;

	void* ptr = arena.allocate_aligned(size, alignment);
	EXPECT_NE(ptr, nullptr);
	EXPECT_TRUE(wmcv::is_ptr_aligned(ptr, alignment));
}

TEST(test_arena_allocator, test_allocator_alloc_too_large)
{
	std::array<std::byte, 4_kB> buffer = {};
	wmcv::ArenaAllocator arena(buffer.data(), buffer.size());

	constexpr size_t size = 8_kB;

	void* ptr = arena.allocate(size);
	EXPECT_EQ(ptr, nullptr);

	EXPECT_EQ(arena.internal_get_marker(), 0);
	EXPECT_EQ(arena.internal_get_size(), 4_kB);
}

TEST(test_arena_allocator, test_allocator_clear)
{
	std::array<std::byte, 4_kB> buffer = {};
	wmcv::ArenaAllocator arena(buffer.data(), buffer.size());

	constexpr size_t size = 2_kB;
	void* ptr = arena.allocate(size);
	EXPECT_NE(ptr, nullptr);

	arena.reset();
	EXPECT_EQ(arena.internal_get_marker(), 0);
}
