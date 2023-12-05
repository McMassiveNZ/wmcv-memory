#ifndef WMCV_BLOCK_ALLOCATOR_H_INCLUDED
#define WMCV_BLOCK_ALLOCATOR_H_INCLUDED

#include "wmcv_memory_block.h"

namespace wmcv
{
	struct BlockFreeListNode
	{
		BlockFreeListNode* next;
	};

	class BlockAllocator
	{
	public:
		BlockAllocator(const Block block, size_t chunkSize, size_t chunkAlignment) noexcept;

		[[nodiscard]] auto allocate() noexcept -> Block;

		void free(void* ptr) noexcept;
		void reset() noexcept;

	private:

		[[nodiscard]] auto owns_address(uintptr_t address) const noexcept -> bool;

		BlockFreeListNode* m_freeStore;
		uintptr_t m_baseAddress;
		size_t m_size;
		size_t m_chunkSize;
	};
}

#endif //WMCV_BLOCK_ALLOCATOR_H_INCLUDED
