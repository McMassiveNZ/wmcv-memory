#include "pch.h"

#include "wmcv_block_allocator.h"
#include "wmcv_allocator_utility.h"

namespace wmcv
{
	BlockAllocator::BlockAllocator(const Block block, size_t chunkSize, size_t chunkAlignment) noexcept
	{
		const auto start = align(block.address, chunkAlignment);
		const auto size = block.size - size_t{start - block.address};

		chunkSize = align(chunkSize, chunkAlignment);

		assert(chunkSize >= sizeof(BlockFreeListNode) && "Chunk size is too small");
		assert(size >= chunkSize && "Memory in block is smaller than chunk size");

		m_baseAddress = block.address;
		m_size = size;
		m_chunkSize = chunkSize;
		m_freeStore = nullptr;

		reset();
	}

	[[nodiscard]] auto BlockAllocator::allocate() noexcept -> Block
	{
		BlockFreeListNode* node = m_freeStore;
		if (node == nullptr)
		{
			return NullBlock();
		}

		m_freeStore = node->next;

		return Block
		{
			.address = ptr_to_address(node),
			.size = m_chunkSize
		};
	}

	void BlockAllocator::free(void* ptr) noexcept
	{
		if (ptr)
		{
			const auto current = ptr_to_address(ptr);

			if (!owns_address(current))
			{
				assert(false && "Memory is out of bounds of the buffer in this pool");
				return;
			}

			auto* node = static_cast<BlockFreeListNode*>(ptr);
			node->next = m_freeStore;
			m_freeStore = node;
		}
	}

	void BlockAllocator::reset() noexcept
	{
		const size_t chunk_count = m_size / m_chunkSize;

		for (size_t i = 0; i < chunk_count; i++)
		{
			const BlockFreeListNode nodeData = {};

			void* ptr = address_to_ptr(m_baseAddress + (i * m_chunkSize));
			std::memcpy(ptr, &nodeData, sizeof(BlockFreeListNode));
			auto* node = static_cast<BlockFreeListNode*>(ptr);

			node->next = m_freeStore;
			m_freeStore = node;
		}
	}

	[[nodiscard]] auto BlockAllocator::owns_address(uintptr_t address) const noexcept -> bool
	{
		return is_address_in_range(address, m_baseAddress, m_size);
	}
}