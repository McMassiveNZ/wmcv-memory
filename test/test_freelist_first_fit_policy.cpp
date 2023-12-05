#include "test_pch.h"
#include "wmcv_freelist_first_fit_policy.h"

#include "wmcv_memory/wmcv_allocator_utility.h"
#include "wmcv_memory/wmcv_memory_block.h"

TEST(test_freelist_allocator, test_allocator_alloc)
{
	std::array<std::byte, 4_kB> memory = {};
	wmcv::Block mem{.address = wmcv::ptr_to_address(memory.data()), .size = memory.size()};
	wmcv::FreeListFirstFitPolicy freelist(mem);

	auto result = freelist.allocate(1_kB);
	EXPECT_NE(result, wmcv::NullBlock());
}

TEST(test_freelist_allocator, test_allocator_free)
{
	std::array<std::byte, 4_kB> memory = {};
	wmcv::Block mem{.address = wmcv::ptr_to_address(memory.data()), .size = memory.size()};
	wmcv::FreeListFirstFitPolicy freeList(mem);

	auto result = freeList.allocate(1_kB);
	EXPECT_NE(result, wmcv::NullBlock());

	auto* ptr = wmcv::address_to_ptr(result.address);
	freeList.free(ptr);

	auto expected = result;
	result = freeList.allocate(1_kB);
	EXPECT_NE(result, wmcv::NullBlock());
	EXPECT_EQ(expected, result);
}

TEST(test_freelist_allocator, test_allocator_alloc_aligned)
{
	std::array<std::byte, 4_kB> memory = {};
	wmcv::Block mem{.address = wmcv::ptr_to_address(memory.data()), .size = memory.size()};
	wmcv::FreeListFirstFitPolicy freeList(mem);

	constexpr size_t alignment = 16;
	constexpr size_t size = 64;

	auto result = freeList.allocate_aligned(size, alignment);
	EXPECT_NE(result, wmcv::NullBlock());
	EXPECT_TRUE(wmcv::is_aligned(result.address, alignment));

	auto* ptr = wmcv::address_to_ptr(result.address);
	freeList.free(ptr);

	auto expected = result;
	result = freeList.allocate_aligned(size, alignment);
	EXPECT_NE(result, wmcv::NullBlock());
	EXPECT_EQ(expected, result);
}

TEST(test_freelist_allocator, test_allocator_alloc_too_large)
{
	std::array<std::byte, 4_kB> memory = {};
	wmcv::Block mem{.address = wmcv::ptr_to_address(memory.data()), .size = memory.size()};
	wmcv::FreeListFirstFitPolicy freeList(mem);

	constexpr size_t size = 8_kB;

	auto result = freeList.allocate(size);
	EXPECT_EQ(result, wmcv::NullBlock());
}

TEST(test_freelist_allocator, test_allocator_oom_from_many_allocs)
{
	std::array<std::byte, 4_kB> memory = {};
	wmcv::Block mem{.address = wmcv::ptr_to_address(memory.data()), .size = memory.size()};
	wmcv::FreeListFirstFitPolicy freeList(mem);

	constexpr size_t size = 1_kB;

	auto result = freeList.allocate(size);
	EXPECT_NE(result, wmcv::NullBlock());

	result = freeList.allocate(size);
	EXPECT_NE(result, wmcv::NullBlock());

	result = freeList.allocate(size);
	EXPECT_NE(result, wmcv::NullBlock());

	result = freeList.allocate(size);
	EXPECT_EQ(result, wmcv::NullBlock());
}

TEST(test_freelist_allocator, test_allocator_reset)
{
	std::array<std::byte, 4_kB> memory = {};
	wmcv::Block mem{.address = wmcv::ptr_to_address(memory.data()), .size = memory.size()};
	wmcv::FreeListFirstFitPolicy freeList(mem);

	constexpr size_t size = 3_kB;
	auto result = freeList.allocate(size);
	EXPECT_NE(result, wmcv::NullBlock());

	result = freeList.allocate(size);
	EXPECT_EQ(result, wmcv::NullBlock());

	freeList.reset();

	result = freeList.allocate(size);
	EXPECT_NE(result, wmcv::NullBlock());
}

TEST(test_freelist_allocator, test_allocator_alloc_twice_and_free_each)
{
	std::array<std::byte, 4_kB> memory = {};
	wmcv::Block mem{.address = wmcv::ptr_to_address(memory.data()), .size = memory.size()};
	wmcv::FreeListFirstFitPolicy freeList(mem);

	constexpr size_t size = 2_kB - 16;

	std::array<wmcv::Block, 2> allocs = {};

	for ( auto& block : allocs )
	{
		block = freeList.allocate(size);
		EXPECT_NE(block, wmcv::NullBlock());
	}

	auto shouldBeNull = freeList.allocate(size);
	EXPECT_EQ(shouldBeNull, wmcv::NullBlock());

	wmcv::Block& block = allocs[1];
	freeList.free(wmcv::address_to_ptr(block.address));
	block = wmcv::NullBlock();

	EXPECT_EQ(block, wmcv::NullBlock());
	block = freeList.allocate(size);
	EXPECT_NE(block, wmcv::NullBlock());

	block = allocs[0];
	freeList.free(wmcv::address_to_ptr(block.address));
	block = wmcv::NullBlock();

	EXPECT_EQ(block, wmcv::NullBlock());
	block = freeList.allocate(size);
	EXPECT_NE(block, wmcv::NullBlock());
}

TEST(test_freelist_allocator, test_allocator_alloc_and_free_multiple_times_interleaved_same_size_odd)
{
	std::array<std::byte, 4_kB> memory = {};
	wmcv::Block mem{.address = wmcv::ptr_to_address(memory.data()), .size = memory.size()};
	wmcv::FreeListFirstFitPolicy freeList(mem);

	constexpr size_t size = 8;

	std::array<wmcv::Block, 128> allocs = {};

	for ( auto& block : allocs )
	{
		block = freeList.allocate(size);
		EXPECT_NE(block, wmcv::NullBlock());
	}

	auto shouldBeNull = freeList.allocate(size);
	EXPECT_EQ(shouldBeNull, wmcv::NullBlock());

	unsigned int count = 0;
	for (auto& block : allocs)
	{
		if (count % 2 != 0)
		{
			freeList.free(wmcv::address_to_ptr(block.address));
			block = wmcv::NullBlock();
		}
		++count;
	}

	count = 0;
	for (auto& block : allocs)
	{
		if (count % 2 != 0)
		{
			EXPECT_EQ(block, wmcv::NullBlock());
			block = freeList.allocate(size);
			EXPECT_NE(block, wmcv::NullBlock());
		}
		++count;
	}
}

TEST(test_freelist_allocator, test_allocator_alloc_and_free_multiple_times_interleaved_same_size_even)
{
	std::array<std::byte, 4_kB> memory = {};
	wmcv::Block mem{.address = wmcv::ptr_to_address(memory.data()), .size = memory.size()};
	wmcv::FreeListFirstFitPolicy freeList(mem);

	constexpr size_t size = 8;

	std::array<wmcv::Block, 128> allocs = {};

	for ( auto& block : allocs )
	{
		block = freeList.allocate(size);
		EXPECT_NE(block, wmcv::NullBlock());
	}

	auto shouldBeNull = freeList.allocate(size);
	EXPECT_EQ(shouldBeNull, wmcv::NullBlock());

	unsigned int count = 0;
	for (auto& block : allocs)
	{
		if (count % 2 == 0)
		{
			freeList.free(wmcv::address_to_ptr(block.address));
			block = wmcv::NullBlock();
		}
		++count;
	}

	count = 0;
	for (auto& block : allocs)
	{
		if (count % 2 == 0)
		{
			EXPECT_EQ(block, wmcv::NullBlock());
			block = freeList.allocate(size);
			EXPECT_NE(block, wmcv::NullBlock());
		}
		++count;
	}
}


TEST(test_freelist_allocator, test_allocator_alloc_and_free_coalesce)
{
	std::array<std::byte, 4_kB> memory = {};
	wmcv::Block mem{.address = wmcv::ptr_to_address(memory.data()), .size = memory.size()};
	wmcv::FreeListFirstFitPolicy freeList(mem);

	constexpr size_t size = 1_kB - 16;
	constexpr size_t large_alloc = 3_kB - 16;

	std::array<wmcv::Block, 4> allocs = {};

	for ( auto& block : allocs )
	{
		block = freeList.allocate(size);
		EXPECT_NE(block, wmcv::NullBlock());
	}

	auto shouldBeNull = freeList.allocate(size);
	EXPECT_EQ(shouldBeNull, wmcv::NullBlock());

	//Free 1st and 3rd allocation to make a free list with 2 blocks
	freeList.free(wmcv::address_to_ptr(allocs[1].address));
	freeList.free(wmcv::address_to_ptr(allocs[3].address));

	//Now free the 2nd Allocation, the free list should make 3 blocks
	//then coalesce them into one large block
	freeList.free(wmcv::address_to_ptr(allocs[2].address));

	{
		auto block = freeList.allocate(large_alloc);
		EXPECT_NE(block, wmcv::NullBlock());
	}

	freeList.reset();

	for ( auto& block : allocs )
	{
		block = freeList.allocate(size);
		EXPECT_NE(block, wmcv::NullBlock());
	}

	//Free 0th and 2nd allocation to make a free list with 2 blocks
	freeList.free(wmcv::address_to_ptr(allocs[0].address));
	freeList.free(wmcv::address_to_ptr(allocs[2].address));

	//Now free the 1st Allocation, the free list should make 3 blocks
	//then coalesce them into one large block
	freeList.free(wmcv::address_to_ptr(allocs[1].address));

	{
		auto block = freeList.allocate(large_alloc);
		EXPECT_NE(block, wmcv::NullBlock());
	}
}

TEST(test_freelist_allocator, test_allocator_alloc_and_free_typical_use_case)
{
	std::array<std::byte, 512> memory = {};
	wmcv::Block mem{.address = wmcv::ptr_to_address(memory.data()), .size = memory.size()};
	wmcv::FreeListFirstFitPolicy freeList(mem);

	std::array<size_t, 8> alloc_sizes = { 38, 122, 16, 80, 172, 62, 10, 12 };
	EXPECT_EQ(std::accumulate(alloc_sizes.begin(), alloc_sizes.end(), 0llu), memory.size());

	const auto block_1 = freeList.allocate(alloc_sizes[0]);
	EXPECT_NE(block_1, wmcv::NullBlock());

	const auto block_2 = freeList.allocate(alloc_sizes[1]);
	EXPECT_NE(block_2, wmcv::NullBlock());

	const auto block_3 = freeList.allocate(alloc_sizes[2]);
	EXPECT_NE(block_3, wmcv::NullBlock());

	const auto block_4 = freeList.allocate(alloc_sizes[4]);
	EXPECT_NE(block_4, wmcv::NullBlock());

	auto block_5 = freeList.allocate(alloc_sizes[4]);
	EXPECT_EQ(block_5, wmcv::NullBlock());

	freeList.free(wmcv::address_to_ptr(block_1.address));
	freeList.free(wmcv::address_to_ptr(block_3.address));
	freeList.free(wmcv::address_to_ptr(block_2.address));

	block_5 = freeList.allocate(alloc_sizes[4]);
	EXPECT_NE(block_5, wmcv::NullBlock());
}
