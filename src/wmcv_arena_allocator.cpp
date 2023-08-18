#include "pch.h"

#include "wmcv_arena_allocator.h"
#include "wmcv_allocator_utility.h"

namespace wmcv
{

static constexpr size_t s_default_alignment = 16;

ArenaAllocator::ArenaAllocator(std::byte* buffer, size_t size) noexcept
	: m_memory(buffer)
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
	const uintptr_t base_address = ptr_to_address(m_memory);
	const uintptr_t curr_ptr = base_address + m_marker;
	const uintptr_t offset = align(curr_ptr, alignment) - base_address;
	const auto memory_resource = std::span(m_memory, m_size);

	if (offset + size <= m_size)
	{
		void* ptr = &memory_resource[offset];
		m_marker = offset + size;

		zero_memory(ptr, size);
		return ptr;
	}

	assert(m_marker != 80);

	return nullptr;
}

void ArenaAllocator::free(void*) noexcept
{
}

void ArenaAllocator::reset() noexcept
{
	m_marker = 0;
}

}