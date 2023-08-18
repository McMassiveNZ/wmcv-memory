#include "test_pch.h"

#include "wmcv_memory/wmcv_allocator_utility.h"

TEST(test_allocator_utility, is_power_of_two_on_power_of_2)
{
	EXPECT_TRUE(wmcv::is_power_of_two(1));
	EXPECT_TRUE(wmcv::is_power_of_two(2));
	EXPECT_TRUE(wmcv::is_power_of_two(4));
	EXPECT_TRUE(wmcv::is_power_of_two(8));
	EXPECT_TRUE(wmcv::is_power_of_two(16));
	EXPECT_TRUE(wmcv::is_power_of_two(32));
	EXPECT_TRUE(wmcv::is_power_of_two(64));
	EXPECT_TRUE(wmcv::is_power_of_two(128));
	EXPECT_TRUE(wmcv::is_power_of_two(256));
}

TEST(test_allocator_utility, is_power_of_two_on_NON_power_of_2)
{
	EXPECT_FALSE(wmcv::is_power_of_two(3));
	EXPECT_FALSE(wmcv::is_power_of_two(5));
	EXPECT_FALSE(wmcv::is_power_of_two(6));
	EXPECT_FALSE(wmcv::is_power_of_two(7));
	EXPECT_FALSE(wmcv::is_power_of_two(9));
	EXPECT_FALSE(wmcv::is_power_of_two(10));
	EXPECT_FALSE(wmcv::is_power_of_two(11));
	EXPECT_FALSE(wmcv::is_power_of_two(12));
	EXPECT_FALSE(wmcv::is_power_of_two(13));
}

TEST(test_allocator_utility, is_aligned_on_aligned)
{
	EXPECT_TRUE(wmcv::is_aligned(1, 1));
	EXPECT_TRUE(wmcv::is_aligned(2, 2));
	EXPECT_TRUE(wmcv::is_aligned(4, 4));
	EXPECT_TRUE(wmcv::is_aligned(8, 4));
	EXPECT_TRUE(wmcv::is_aligned(16, 8));
	EXPECT_TRUE(wmcv::is_aligned(32, 16));
	EXPECT_TRUE(wmcv::is_aligned(64, 16));
	EXPECT_TRUE(wmcv::is_aligned(128, 16));
	EXPECT_TRUE(wmcv::is_aligned(256, 16));
}

TEST(test_allocator_utility, is_aligned_on_NON_aligned)
{
	EXPECT_FALSE(wmcv::is_aligned(3, 16));
	EXPECT_FALSE(wmcv::is_aligned(5, 16));
	EXPECT_FALSE(wmcv::is_aligned(6, 16));
	EXPECT_FALSE(wmcv::is_aligned(7, 16));
	EXPECT_FALSE(wmcv::is_aligned(9, 16));
	EXPECT_FALSE(wmcv::is_aligned(10, 16));
	EXPECT_FALSE(wmcv::is_aligned(11, 16));
	EXPECT_FALSE(wmcv::is_aligned(12, 16));
	EXPECT_FALSE(wmcv::is_aligned(13, 16));
}

TEST(test_allocator_utility, test_align_round_up)
{
	EXPECT_EQ(wmcv::align(size_t{3}, size_t{4}), size_t{4});
	EXPECT_EQ(wmcv::align(size_t{3}, size_t{16}), size_t{16});
	EXPECT_EQ(wmcv::align(size_t{9}, size_t{16}), size_t{16});
	EXPECT_EQ(wmcv::align(size_t{64}, size_t{16}), size_t{64});
	EXPECT_EQ(wmcv::align(size_t{65}, size_t{16}), size_t{80});
}
