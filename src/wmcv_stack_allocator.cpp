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

static_assert(std::is_trivial_v<StackAllocationHeader>,
	"Allocation Header needs to be trivial so it can be memcpy into the raw bytes");

StackAllocator::StackAllocator(Block block) noexcept
	: m_baseAddress(block.address)
	, m_size(block.size)
	, m_previousMarker(0llu)
	, m_currentMarker(0llu)
{
}

auto StackAllocator::allocate(size_t size) noexcept -> Block
{
	constexpr size_t s_default_alignment = 16;
	return allocate_aligned(size, s_default_alignment);
}

constexpr static size_t calc_padding_with_header(uintptr_t ptr, uintptr_t alignment) noexcept
{
	assert(is_power_of_two(alignment));

	const uintptr_t remainder = ptr & (alignment - 1);

	uintptr_t padding = 0;
	uintptr_t needed_space = 0;

	if (remainder != 0)
	{
		padding = alignment - remainder;
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

auto StackAllocator::allocate_aligned(size_t size, size_t alignment) noexcept -> Block
{
	assert(is_power_of_two(alignment));
	assert(alignment <= 128 && "Padding is stored in a byte so alignment cannot exceed 128");

	const uintptr_t current_address = m_baseAddress + m_currentMarker;
	const size_t padding = calc_padding_with_header(current_address, uintptr_t{alignment});
	if (m_currentMarker + padding + size > m_size)
	{
		return NullBlock();
	}

	m_previousMarker = m_currentMarker;
	m_currentMarker += padding;

	const uintptr_t address = current_address + padding;

	const StackAllocationHeader header
	{
		.previousOffset = m_previousMarker, 
		.padding = padding
	};

	auto* const header_address = address_to_ptr(address - sizeof(StackAllocationHeader));
	std::memcpy(header_address, &header, sizeof(StackAllocationHeader));

	m_currentMarker += size;
	return { .address = address, .size = size };
}

void StackAllocator::free(void* ptr) noexcept
{
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

	StackAllocationHeader header = {};
	const uintptr_t header_address = current_address - sizeof(StackAllocationHeader);
	std::memcpy(&header, address_to_ptr(header_address), sizeof(StackAllocationHeader));

	const size_t previous_offset = current_address - header.padding - m_baseAddress;
	if (previous_offset != header.previousOffset)
	{
		assert(false && "Out of order stack allocator free");
		return;
	}

	m_currentMarker = m_previousMarker;
	m_previousMarker = header.previousOffset;
}

void StackAllocator::reset() noexcept
{
	m_currentMarker = 0llu;
	m_previousMarker = 0llu;
}

auto StackAllocator::owns_address(uintptr_t address) const noexcept -> bool
{
	const uintptr_t start = m_baseAddress;
	const uintptr_t end = start + uintptr_t{m_size};

	return start <= address && address < end;
}

} // namespace wmcv
