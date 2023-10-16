#include "pch.h"

#include "wmcv_pool_allocator.h"
#include "wmcv_allocator_utility.h"

namespace wmcv
{
	struct PoolFreeListNode
	{
		PoolFreeListNode* next;
	};

	PoolAllocator::PoolAllocator(const Block block, size_t chunkSize, size_t chunkAlignment) noexcept
	{
		const auto start = align(block.address, chunkAlignment);
		const auto size = block.size - size_t{start - block.address};

		chunkSize = align(chunkSize, chunkAlignment);

		assert(chunkSize >= sizeof(PoolFreeListNode) && "Chunk size is too small");
		assert(size >= chunkSize && "Memory in block is smaller than chunk size");

		m_baseAddress = block.address;
		m_size = size;
		m_chunkSize = chunkSize;
		m_freeStore = nullptr;

		reset();
	}

	[[nodiscard]] auto PoolAllocator::allocate() noexcept -> Block
	{
		PoolFreeListNode* node = static_cast<PoolFreeListNode*>(m_freeStore);
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

	void PoolAllocator::free(void* ptr) noexcept
	{
		if (ptr)
		{
			const auto current = ptr_to_address(ptr);

			if (!owns_address(current))
			{
				assert(false && "Memory is out of bounds of the buffer in this pool");
				return;
			}

			auto* node = static_cast<PoolFreeListNode*>(ptr);
			node->next = static_cast<PoolFreeListNode*>(m_freeStore);
			m_freeStore = node;
		}
	}

	void PoolAllocator::reset() noexcept
	{
		const size_t chunk_count = m_size / m_chunkSize;

		for (size_t i = 0; i < chunk_count; i++)
		{
			void* ptr = address_to_ptr(m_baseAddress + (i * m_chunkSize));
			auto* node = static_cast<PoolFreeListNode*>(ptr);

			node->next = static_cast<PoolFreeListNode*>(m_freeStore);
			m_freeStore = node;
		}
	}

	[[nodiscard]] auto PoolAllocator::owns_address(uintptr_t address) const noexcept -> bool
	{
		const uintptr_t start = m_baseAddress;
		const uintptr_t end = start + uintptr_t{m_size};

		return start <= address && address < end;
	}
}