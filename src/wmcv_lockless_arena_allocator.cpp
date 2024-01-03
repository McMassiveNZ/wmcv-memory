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
	size_t expected{m_marker.load()}; 
	size_t desired{0};

	do
	{
		const uintptr_t curr_ptr = m_baseAddress + expected;
		const uintptr_t offset = align(curr_ptr, alignment) - m_baseAddress;
		desired = expected + offset + size;
	}
	while (!m_marker.compare_exchange_weak(expected, desired));

	if ( desired <= m_size)
	{
		const uintptr_t offset = desired - expected - size;
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