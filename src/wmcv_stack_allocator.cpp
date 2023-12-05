#include "pch.h"

#include "wmcv_stack_allocator.h"
#include "wmcv_allocator_utility.h"
#include "wmcv_allocator_padding.h"

namespace wmcv
{

struct StackAllocationHeader
{
	size_t previousOffset;
	size_t padding;
};

static_assert(std::is_standard_layout_v<StackAllocationHeader>,
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

auto StackAllocator::allocate_aligned(size_t size, size_t alignment) noexcept -> Block
{
	assert(is_power_of_two(alignment));
	assert(alignment <= 128 && "Padding is stored in a byte so alignment cannot exceed 128");

	const uintptr_t current_address = m_baseAddress + m_currentMarker;
	const size_t padding = compute_padding(current_address, uintptr_t{alignment}, sizeof(StackAllocationHeader));
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
	return is_address_in_range(address, m_baseAddress, m_size);
}

} // namespace wmcv
