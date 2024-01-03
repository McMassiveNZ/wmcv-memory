#include "test_pch.h"

#include "wmcv_memory/wmcv_lockless_block_allocator.h"
#include "wmcv_memory/wmcv_allocator_utility.h"

TEST(test_lockless_block_allocator, test_allocator_alloc)
{
	std::array<std::byte, 4_kB> buffer = {};
	wmcv::Block mem{.address = wmcv::ptr_to_address(buffer.data()), .size = buffer.size()};
	wmcv::LocklessBlockAllocator pool(mem, 32, 4);

	auto result = pool.allocate();
	EXPECT_NE(result, wmcv::NullBlock());
	EXPECT_EQ(result.size, 32);
}

TEST(test_lockless_block_allocator, test_allocator_alloc_from_multiple_threads)
{
	constexpr size_t chunk_alignment = 4;
	alignas(chunk_alignment) std::array<std::byte, 4_kB> buffer = {};
	wmcv::Block mem{.address = wmcv::ptr_to_address(buffer.data()), .size = buffer.size()};

	const size_t num_threads = std::thread::hardware_concurrency();
	const size_t chunk_size = buffer.size() / num_threads;
	wmcv::LocklessBlockAllocator pool(mem, chunk_size, chunk_alignment);

	std::vector<std::jthread> threads;
	std::vector<wmcv::Block> blocks(num_threads);

	for (size_t i = 0; i < num_threads; ++i)
	{
		threads.emplace_back([&pool, &blocks, i]
		{
			auto block = pool.allocate();
			blocks[i] = block;
		});
	}

	threads.clear();

	std::sort(blocks.begin(), blocks.end());

	for ( size_t i = 0; i < blocks.size() - 1; ++i )
	{
		const auto B0 = blocks[i + 0];
		const auto B1 = blocks[i + 1];

		EXPECT_NE(B0, wmcv::NullBlock());

		const uintptr_t next = B1.address;
		const uintptr_t prev = B0.address + B0.size;
		EXPECT_EQ(prev, next);
	}
}

TEST(test_lockless_block_allocator, test_allocator_alloc_blocks_are_aligned)
{
	std::array<std::byte, 1_kB> buffer = {};
	wmcv::Block mem{.address = wmcv::ptr_to_address(buffer.data()), .size = buffer.size()};
	wmcv::LocklessBlockAllocator pool(mem, 64, 8);

	auto result = pool.allocate();
	EXPECT_NE(result, wmcv::NullBlock());
	EXPECT_TRUE(wmcv::is_aligned(result.address, 8));
}

TEST(test_lockless_block_allocator, test_allocator_alloc_not_enough_space)
{
	std::array<std::byte, 4_kB> buffer = {};
	wmcv::Block mem{.address = wmcv::ptr_to_address(buffer.data()), .size = buffer.size()};
	wmcv::LocklessBlockAllocator pool(mem, 4_kB, 4);

	auto result = pool.allocate();
	result = pool.allocate();
	EXPECT_EQ(result, wmcv::NullBlock());
}

TEST(test_lockless_block_allocator, test_allocator_alloc_not_enough_space_from_multiple_threads)
{
	constexpr size_t chunk_alignment = 16;
	alignas(16) std::array<std::byte, 4_kB> buffer = {};
	wmcv::Block mem{.address = wmcv::ptr_to_address(buffer.data()), .size = buffer.size()};
	wmcv::LocklessBlockAllocator pool(mem, 1_kB, chunk_alignment);

	std::vector<std::jthread> threads;
	std::vector<wmcv::Block> blocks(5);

	for (size_t i = 0; i < blocks.size(); ++i)
	{
		threads.emplace_back([&pool, &blocks, i]
		{
			auto block = pool.allocate();
			blocks[i] = block;
		});
	}

	threads.clear();

	const size_t result = std::count(blocks.begin(), blocks.end(), wmcv::NullBlock());

	EXPECT_EQ(result, 1);

	std::sort(blocks.begin(), blocks.end());

	for ( size_t i = 1; i < blocks.size() - 1; ++i )
	{
		const auto B0 = blocks[i + 0];
		const auto B1 = blocks[i + 1];

		EXPECT_NE(B0, wmcv::NullBlock());

		const uintptr_t next = B1.address;
		const uintptr_t prev = B0.address + B0.size;
		EXPECT_EQ(prev, next);
	}
}

TEST(test_lockless_block_allocator, test_allocator_clear)
{
	std::array<std::byte, 4_kB> buffer = {};
	wmcv::Block mem{.address = wmcv::ptr_to_address(buffer.data()), .size = buffer.size()};
	wmcv::LocklessBlockAllocator pool(mem, 4_kB, 4);

	auto block = pool.allocate();
	EXPECT_NE(block, wmcv::NullBlock());

	block = pool.allocate();
	EXPECT_EQ(block, wmcv::NullBlock());

	pool.reset();

	block = pool.allocate();
	EXPECT_NE(block, wmcv::NullBlock());
}