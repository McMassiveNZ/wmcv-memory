#include "pch.h"
#include "wmcv_buddy_allocator.h"
#include "wmcv_allocator_utility.h"

namespace wmcv
{

static_assert(is_power_of_two(sizeof(BuddyBlock)), "Buddy Block Header must be power of 2");

static BuddyBlock* Next(BuddyBlock* block) noexcept
{
	return static_cast<BuddyBlock*>(offset_ptr(block, block->size));
}

static void WriteBuddyHeader(void* ptr, size_t size) noexcept
{
	const BuddyBlock blockData = {.size = size, .free = true};
	std::memcpy(ptr, &blockData, sizeof(BuddyBlock));
}

static constexpr auto ComputeSize(size_t alignment, size_t size) noexcept -> size_t
{
    size_t actual_size = alignment;
    
    size += sizeof(BuddyBlock);
	size = align(uintptr_t{size}, alignment); 
    
    while (size > actual_size) 
	{
        actual_size *= 2;
    }
    
    return actual_size;
}

static constexpr auto ComputeAlignment(size_t alignment) noexcept -> size_t
{
	if (alignment < sizeof(BuddyBlock))
	{
		alignment = sizeof(BuddyBlock);
	}

	return alignment;
}

BuddyAllocator::BuddyAllocator(const Block block, size_t alignment) noexcept
	: m_baseAddress(block.address)
	, m_size(block.size)
	, m_alignment(ComputeAlignment(alignment))
{
	assert(m_baseAddress != 0llu && "Base address is null");
	assert(is_power_of_two(m_size) && "Size is not a power-of-two");
	assert(is_power_of_two(m_alignment) && "Alignment is not a power-of-two");
	assert(m_baseAddress % m_alignment == 0 && "data is not aligned to minimum alignment");

	reset();
}

[[nodiscard]] auto BuddyAllocator::allocate(size_t size) noexcept -> Block
{
	if (size < m_alignment)
	{
		size = m_alignment;
	}

	const size_t actual_size = ComputeSize(m_alignment, size);

	BuddyBlock* found = search_blocks(actual_size);
	if (!found)
	{
		coalesce();
		found = search_blocks(actual_size);
	}

	if (found && offset_ptr(found, m_alignment) != m_tail)
	{
		found->free = false;
		return Block
		{
			.address = ptr_to_address(found) + m_alignment,
			.size = size
		};
	}

	return NullBlock();
}

void BuddyAllocator::free(void* ptr) noexcept
{
	if (ptr)
	{
		assert(m_head <= ptr);
		assert(ptr < m_tail);

		auto* block = static_cast<BuddyBlock*>(offset_ptr_back(ptr, m_alignment));
		block->free = true;
	}
}

void BuddyAllocator::reset() noexcept
{
	const BuddyBlock blockData = {.size = m_size, .free = true};
	void* ptr = address_to_ptr(m_baseAddress);
	std::memcpy(ptr, &blockData, sizeof(BuddyBlock));
	m_head = static_cast<BuddyBlock*>(ptr);
	m_tail = Next(m_head);
}

[[nodiscard]] auto BuddyAllocator::owns_address(uintptr_t address) const noexcept -> bool
{
	return is_address_in_range(address, m_baseAddress, m_size);
}

[[nodiscard]] auto BuddyAllocator::split_block(BuddyBlock* block, size_t size) noexcept -> BuddyBlock*
{
	if (block && size > 0)
	{
		while (size < block->size)
		{
			const size_t sz = block->size / 2;
			block->size = sz;

			auto* buddy = Next(block);
			WriteBuddyHeader(buddy, sz);
		}

		if (size <= block->size)
		{
			return block;
		}
	}

	return nullptr;
}

[[nodiscard]] auto BuddyAllocator::search_blocks(size_t size) noexcept -> BuddyBlock*
{
	BuddyBlock* best = nullptr;
	BuddyBlock* left = m_head;
	BuddyBlock* right = Next(left);

	if (right == m_tail && left->free)
	{
		return split_block(left, size);
	}

	while (left < m_tail && right < m_tail)
	{
		if (left->free && right->free && left->size == right->size)
		{
			left->size *= 2;
			if (size <= left->size && (!best || left->size <= best->size))
			{
				best = left;
			}

			left = Next(right);
			if (left < m_tail)
			{
				right = Next(left);
			}
			continue;
		}


		if (right->free && size <= right->size && (!best || right->size < best->size))
		{
			best = right;
		}
		else if (left->free && size <= left->size && (!best || left->size <= best->size))
		{
			best = left;
		}

		if (left->size <= right->size)
		{
			left = Next(right);
			if (left < m_tail)
			{
				right = Next(left);

				if (right == m_tail && left->free && size <= left->size && (!best || left->size <= best->size))
				{
					best = left;
				}
			}
		}
		else
		{
			left = std::exchange(right, Next(right));
		}
	}

	return split_block(best, size);
}


void BuddyAllocator::coalesce() noexcept
{
	while (true)
	{
		BuddyBlock* block = m_head;
		BuddyBlock* buddy = Next(block);

		bool did_coalesce = false;
		while (block < m_tail && buddy < m_tail)
		{
			if (block->free && buddy->free && block->size == buddy->size)
			{
				block->size <<= 1;
				block = Next(block);
				if (block < m_tail)
				{
					buddy = Next(block);
					did_coalesce = true;
				}
			}
			else if (block->size < buddy->size)
			{
				block = buddy;
				buddy = Next(buddy);
			}
			else
			{
				block = Next(buddy);
				if (block < m_tail)
				{
					buddy = Next(block);
				}
			}
		}

		if (!did_coalesce)
		{
			return;
		}
	}
}

}