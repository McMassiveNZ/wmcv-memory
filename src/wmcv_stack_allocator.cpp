#include "pch.h"

#include "wmcv_stack_allocator.h"
#include "wmcv_allocator_utility.h"

namespace wmcv
{

struct StackAllocationHeader
{
	size_t previousOffset;
	size_t padding;
};

StackAllocator::StackAllocator(std::byte* buffer, size_t size) noexcept
	: m_baseAddress(ptr_to_address(buffer))
	, m_size(size)
	, m_previousMarker(0llu)
	, m_currentMarker(0llu)
{
}

auto StackAllocator::allocate(size_t size) noexcept -> void*
{
	constexpr size_t s_default_alignment = 16;
	return allocate_aligned(size, s_default_alignment);
}

static size_t calc_padding_with_header(uintptr_t ptr, uintptr_t alignment) noexcept
{
	assert(is_power_of_two(alignment));

	const uintptr_t modulo = ptr & (alignment - 1);

	uintptr_t padding = 0;
	uintptr_t needed_space = 0;

	if (modulo != 0)
	{
		padding = alignment - modulo;
	}

	needed_space = sizeof(StackAllocationHeader);

	if (padding < needed_space)
	{
		needed_space -= padding;

		if ((needed_space & (alignment - 1)) != 0)
		{
			padding += alignment * (1 + (needed_space / alignment));
		}
		else
		{
			padding += alignment * (needed_space / alignment);
		}
	}

	return padding;
}

auto StackAllocator::allocate_aligned(size_t size, size_t alignment) noexcept -> void*
{
	assert(is_power_of_two(alignment));
	assert(alignment <= 128 && "Padding is stored in a byte so alignment cannot exceed 128");

	const uintptr_t current_address = m_baseAddress + m_currentMarker;
	const size_t padding = calc_padding_with_header(current_address, uintptr_t{alignment});
	if (m_currentMarker + padding + size > m_size)
	{
		return nullptr;
	}

	m_previousMarker = m_currentMarker;
	m_currentMarker += padding;

	const uintptr_t next_addr = current_address + padding;
	void* header_address = address_to_ptr(next_addr - sizeof(StackAllocationHeader));
	auto* const header = static_cast<StackAllocationHeader*>(header_address);
	header->padding = padding;
	header->previousOffset = m_previousMarker;

	m_currentMarker += size;
	void* result = address_to_ptr(next_addr);
	zero_memory(result, size);

	return result;
}

void StackAllocator::free(void* ptr) noexcept
{
	static_assert(std::is_same_v<uintptr_t, size_t>);

	if (ptr == nullptr)
		return;

	const uintptr_t current_address = ptr_to_address(ptr);

	if (!owns_address(current_address))
	{
		assert(false && "Out of bounds memory address passed to stack allocator (free)");
		return;
	}

	if (current_address >= m_baseAddress + uintptr_t{m_currentMarker})
	{
		assert(false && "Double free");
		return;
	}

	const uintptr_t header_location = current_address - sizeof(StackAllocationHeader);
	const auto* header = static_cast<StackAllocationHeader*>(address_to_ptr(header_location));
	const size_t previous_offset = current_address - header->padding - m_baseAddress;

	if (previous_offset != header->previousOffset)
	{
		assert(0 && "Out of order stack allocator free");
		return;
	}

	m_currentMarker = m_previousMarker;
	m_previousMarker = header->previousOffset;
}

void StackAllocator::reset() noexcept
{
	m_currentMarker = 0llu;
	m_previousMarker = 0llu;
}

bool StackAllocator::owns_address(uintptr_t address) const noexcept
{
	const uintptr_t start = m_baseAddress;
	const uintptr_t end = start + uintptr_t{m_size};

	return start <= address && address < end;
}

} // namespace wmcv
