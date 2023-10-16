#include "test_pch.h"

#include "wmcv_memory/wmcv_pool_allocator.h"
#include "wmcv_memory/wmcv_allocator_utility.h"

TEST(test_pool_allocator, test_allocator_alloc)
{
	std::array<std::byte, 4_kB> buffer = {};
	wmcv::Block mem{.address = wmcv::ptr_to_address(buffer.data()), .size = buffer.size()};
	wmcv::PoolAllocator pool(mem, 32, 4);

	auto result = pool.allocate();
	EXPECT_NE(result, wmcv::NullBlock());
	EXPECT_EQ(result.size, 32);
}

TEST(test_pool_allocator, test_allocator_alloc_blocks_are_aligned)
{
	std::array<std::byte, 1_kB> buffer = {};
	wmcv::Block mem{.address = wmcv::ptr_to_address(buffer.data()), .size = buffer.size()};
	wmcv::PoolAllocator pool(mem, 64, 8);

	auto result = pool.allocate();
	EXPECT_NE(result, wmcv::NullBlock());
	EXPECT_TRUE(wmcv::is_aligned(result.address, 8));
}

TEST(test_pool_allocator, test_allocator_alloc_not_enough_space)
{
	std::array<std::byte, 4_kB> buffer = {};
	wmcv::Block mem{.address = wmcv::ptr_to_address(buffer.data()), .size = buffer.size()};
	wmcv::PoolAllocator pool(mem, 4_kB, 4);

	auto result = pool.allocate();
	result = pool.allocate();
	EXPECT_EQ(result, wmcv::NullBlock());
}

TEST(test_pool_allocator, test_allocator_clear)
{
	std::array<std::byte, 4_kB> buffer = {};
	wmcv::Block mem{.address = wmcv::ptr_to_address(buffer.data()), .size = buffer.size()};
	wmcv::PoolAllocator pool(mem, 4_kB, 4);

	auto block = pool.allocate();
	EXPECT_NE(block, wmcv::NullBlock());

	block = pool.allocate();
	EXPECT_EQ(block, wmcv::NullBlock());

	pool.reset();

	block = pool.allocate();
	EXPECT_NE(block, wmcv::NullBlock());
}