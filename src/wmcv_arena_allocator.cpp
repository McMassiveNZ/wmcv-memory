#include "pch.h"

#include "wmcv_arena_allocator.h"
#include "wmcv_allocator_utility.h"

namespace wmcv
{

ArenaAllocator::ArenaAllocator(Block block) noexcept
	: m_baseAddress(block.address)
	, m_size(block.size)
	, m_marker(0llu)
{
}

auto ArenaAllocator::allocate(size_t size) noexcept -> Block
{
	constexpr size_t s_default_alignment = 16;
	return allocate_aligned(size, s_default_alignment);
}

auto ArenaAllocator::allocate_aligned(size_t size, size_t alignment) noexcept -> Block
{
	const uintptr_t curr_ptr = m_baseAddress + m_marker;
	const uintptr_t offset = align(curr_ptr, alignment) - m_baseAddress;

	if (offset + size <= m_size)
	{
		const uintptr_t address = m_baseAddress + offset;
		m_marker = offset + size;
		return { .address = address, .size = size };
	}

	return NullBlock();
}

void ArenaAllocator::free(void*) noexcept
{
}

void ArenaAllocator::reset() noexcept
{
	m_marker = 0llu;
}

}