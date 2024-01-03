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
	wmcv::Block mem{.address = 0x00040000, .size = 4_MB};
	wmcv::LocklessArenaAllocator arena(mem);

	const size_t num_threads = std::thread::hardware_concurrency();
	const size_t alloc_size = (1_kB / num_threads);

	std::vector<std::thread> threads;
	std::vector<wmcv::Block> blocks(num_threads);

	for (size_t i = 0; i < num_threads; ++i)
	{
		auto& t = threads.emplace_back([&arena, &blocks, i, alloc_size]
		{
			auto block = arena.allocate(alloc_size);
			blocks[i] = block;
		});

		t.join();
	}

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