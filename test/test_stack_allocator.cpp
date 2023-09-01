#include "test_pch.h"
#include "wmcv_memory/wmcv_allocator_utility.h"
#include "wmcv_memory/wmcv_stack_allocator.h"

TEST(test_stack_allocator, test_allocator_alloc)
{
	std::array<std::byte, 4_kB> memory = {};
	wmcv::Block mem{.address = wmcv::ptr_to_address(memory.data()), .size = memory.size()};
	wmcv::StackAllocator stack(mem);

	auto result = stack.allocate(1_kB);
	EXPECT_NE(result, wmcv::NullBlock());
}

TEST(test_stack_allocator, test_allocator_free)
{
	std::array<std::byte, 4_kB> memory = {};
	wmcv::Block mem{.address = wmcv::ptr_to_address(memory.data()), .size = memory.size()};
	wmcv::StackAllocator stack(mem);

	auto result = stack.allocate(1_kB);
	EXPECT_NE(result, wmcv::NullBlock());

	auto* ptr = reinterpret_cast<void*>(result.address);
	stack.free(ptr);

	auto expected = result;
	result = stack.allocate(1_kB);
	EXPECT_NE(result, wmcv::NullBlock());
	EXPECT_EQ(expected, result);
}

TEST(test_stack_allocator, test_allocator_alloc_aligned)
{
	std::array<std::byte, 4_kB> memory = {};
	wmcv::Block mem{.address = wmcv::ptr_to_address(memory.data()), .size = memory.size()};
	wmcv::StackAllocator stack(mem);

	constexpr size_t alignment = 32;
	constexpr size_t size = 64;

	auto result = stack.allocate_aligned(size, alignment);
	EXPECT_NE(result, wmcv::NullBlock());
	EXPECT_TRUE(wmcv::is_aligned(result.address, alignment));

	auto* ptr = reinterpret_cast<void*>(result.address);
	stack.free(ptr);

	auto expected = result;
	result = stack.allocate_aligned(size, alignment);
	EXPECT_NE(result, wmcv::NullBlock());
	EXPECT_EQ(expected, result);
}

TEST(test_stack_allocator, test_allocator_alloc_too_large)
{
	std::array<std::byte, 4_kB> memory = {};
	wmcv::Block mem{.address = wmcv::ptr_to_address(memory.data()), .size = memory.size()};
	wmcv::StackAllocator stack(mem);

	constexpr size_t size = 8_kB;

	auto result = stack.allocate(size);
	EXPECT_EQ(result, wmcv::NullBlock());
}

TEST(test_stack_allocator, test_allocator_clear)
{
	std::array<std::byte, 4_kB> memory = {};
	wmcv::Block mem{.address = wmcv::ptr_to_address(memory.data()), .size = memory.size()};
	wmcv::StackAllocator stack(mem);

	constexpr size_t size = 3_kB;
	auto result = stack.allocate(size);
	EXPECT_NE(result, wmcv::NullBlock());

	result = stack.allocate(size);
	EXPECT_EQ(result, wmcv::NullBlock());

	stack.reset();

	result = stack.allocate(size);
	EXPECT_NE(result, wmcv::NullBlock());
}
