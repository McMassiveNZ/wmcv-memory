#include "pch.h"

#include "wmcv_arena_allocator.h"
#include "wmcv_allocator_utility.h"

namespace wmcv
{

static constexpr size_t s_default_alignment = 16;

ArenaAllocator::ArenaAllocator(std::byte* buffer, size_t size) noexcept
	: m_baseAddress(ptr_to_address(buffer))
	, m_size(size)
	, m_marker(0llu)
{
}

auto ArenaAllocator::allocate(size_t size) noexcept -> void*
{
	return allocate_aligned(size, s_default_alignment);
}

auto ArenaAllocator::allocate_aligned(size_t size, size_t alignment) noexcept -> void*
{
	const uintptr_t curr_ptr = m_baseAddress + m_marker;
	const uintptr_t offset = align(curr_ptr, alignment) - m_baseAddress;

	if (offset + size <= m_size)
	{
		const uintptr_t address = m_baseAddress + offset;
		m_marker = offset + size;

		void* ptr = address_to_ptr(address);
		zero_memory(ptr, size);
		return ptr;
	}

	return nullptr;
}

void ArenaAllocator::free(void*) noexcept
{
}

void ArenaAllocator::reset() noexcept
{
	m_marker = 0llu;
}

}