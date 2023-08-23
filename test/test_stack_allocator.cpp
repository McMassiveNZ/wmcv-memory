#include "test_pch.h"
#include "wmcv_memory/wmcv_allocator_utility.h"
#include "wmcv_memory/wmcv_stack_allocator.h"

TEST(test_stack_allocator, test_construct_allocator)
{
	std::array<std::byte, 4_kB> buffer;
	wmcv::StackAllocator stack(buffer.data(), buffer.size());

	EXPECT_EQ(stack.internal_get_current_marker(), 0);
	EXPECT_EQ(stack.internal_get_previous_marker(), 0);
	EXPECT_EQ(stack.internal_get_size(), 4_kB);
}

TEST(test_stack_allocator, test_allocator_alloc)
{
	std::array<std::byte, 4_kB> buffer;
	wmcv::StackAllocator stack(buffer.data(), buffer.size());

	void* ptr = stack.allocate(1_kB);
	EXPECT_NE(ptr, nullptr);

	EXPECT_EQ(stack.internal_get_previous_marker(), 0ull);
	EXPECT_EQ(stack.internal_get_size(), 4_kB);
}

TEST(test_stack_allocator, test_allocator_free)
{
	std::array<std::byte, 4_kB> buffer;
	wmcv::StackAllocator stack(buffer.data(), buffer.size());

	void* ptr = stack.allocate(1_kB);
	EXPECT_NE(ptr, nullptr);

	EXPECT_EQ(stack.internal_get_previous_marker(), 0ull);
	EXPECT_EQ(stack.internal_get_size(), 4_kB);

	stack.free(ptr);

	EXPECT_EQ(stack.internal_get_current_marker(), 0ull);
	EXPECT_EQ(stack.internal_get_previous_marker(), 0ull);
}

TEST(test_stack_allocator, test_allocator_alloc_aligned)
{
	std::array<std::byte, 4_kB> buffer = {};
	wmcv::StackAllocator stack(buffer.data(), buffer.size());

	constexpr size_t alignment = 32;
	constexpr size_t size = 64;

	void* ptr = stack.allocate_aligned(size, alignment);
	EXPECT_NE(ptr, nullptr);

	EXPECT_EQ(stack.internal_get_previous_marker(), 0);
	EXPECT_EQ(stack.internal_get_size(), 4_kB);

	EXPECT_TRUE(wmcv::is_ptr_aligned(ptr, alignment));
}

TEST(test_stack_allocator, test_allocator_alloc_too_large)
{
	std::array<std::byte, 4_kB> buffer;
	wmcv::StackAllocator stack(buffer.data(), buffer.size());

	constexpr size_t size = 8_kB;

	void* ptr = stack.allocate(size);
	EXPECT_EQ(ptr, nullptr);

	EXPECT_EQ(stack.internal_get_current_marker(), 0);
	EXPECT_EQ(stack.internal_get_previous_marker(), 0);
	EXPECT_EQ(stack.internal_get_size(), 4_kB);
}

TEST(test_stack_allocator, test_allocator_clear)
{
	std::array<std::byte, 4_kB> buffer;
	wmcv::StackAllocator stack(buffer.data(), buffer.size());

	constexpr size_t size = 2_kB;
	void* ptr = stack.allocate(size);
	EXPECT_NE(ptr, nullptr);

	stack.reset();
	EXPECT_EQ(stack.internal_get_current_marker(), 0);
	EXPECT_EQ(stack.internal_get_previous_marker(), 0);
}
