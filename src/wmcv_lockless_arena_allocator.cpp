#include "pch.h"

#include "wmcv_lockless_arena_allocator.h"
#include "wmcv_allocator_utility.h"

namespace wmcv
{

LocklessArenaAllocator::LocklessArenaAllocator(Block block) noexcept
	: m_baseAddress(block.address)
	, m_size(block.size)
	, m_marker(0llu)
{
}

auto LocklessArenaAllocator::allocate(size_t size) noexcept -> Block
{
	constexpr size_t s_default_alignment = 16;
	return allocate_aligned(size, s_default_alignment);
}

auto LocklessArenaAllocator::allocate_aligned(size_t size, size_t alignment) noexcept -> Block
{
	const uintptr_t curr_ptr = m_baseAddress + m_marker.load(std::memory_order_relaxed);
	const uintptr_t offset = align(curr_ptr, alignment) - m_baseAddress;
	const size_t alloc_size = offset + size;
	const size_t old_marker = m_marker.fetch_add(alloc_size, std::memory_order_acq_rel);

	if ( old_marker + alloc_size <= m_size)
	{
		const uintptr_t address = m_baseAddress + offset;
		return { .address = address, .size = size };
	}

	return NullBlock();
}

void LocklessArenaAllocator::free(void*) noexcept
{
}

void LocklessArenaAllocator::reset() noexcept
{
	m_marker = 0llu;
}

}