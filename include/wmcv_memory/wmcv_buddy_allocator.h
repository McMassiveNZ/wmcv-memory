#ifndef WMCV_BUDDY_ALLOCATOR_H_INCLUDED
#define WMCV_BUDDY_ALLOCATOR_H_INCLUDED

#include "wmcv_memory_block.h"

namespace wmcv
{
	struct BuddyBlock
	{
		size_t size : 63;
		size_t free : 1;
	};

	class BuddyAllocator
	{
	public:
		BuddyAllocator(const Block block, size_t alignment = sizeof(BuddyBlock)) noexcept;

		[[nodiscard]] auto allocate(size_t size) noexcept -> Block;

		void free(void* ptr) noexcept;
		void reset() noexcept;

	private:

		[[nodiscard]] auto owns_address(uintptr_t address) const noexcept -> bool;
		[[nodiscard]] auto split_block(BuddyBlock* block, size_t size) noexcept -> BuddyBlock*;
		[[nodiscard]] auto search_blocks(size_t size) noexcept -> BuddyBlock*;
		void coalesce() noexcept;

		uintptr_t m_baseAddress;
		size_t m_size;
		size_t m_alignment;

		BuddyBlock* m_head;
		BuddyBlock* m_tail;
	};
}

#endif //WMCV_BUDDY_ALLOCATOR_H_INCLUDED
