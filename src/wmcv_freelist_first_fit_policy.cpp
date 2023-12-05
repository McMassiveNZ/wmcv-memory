#include "pch.h"
#include "wmcv_freelist_first_fit_policy.h"
#include "wmcv_allocator_utility.h"
#include "wmcv_allocator_padding.h"

namespace wmcv
{

struct FreeListAllocationHeader
{
	size_t block_size;
	size_t padding;
};

static_assert(std::is_standard_layout_v<FreeListAllocationHeader>, "FreeListAllocationHeader must be POD");

static void sanity_check_nodes([[maybe_unused]] void* before, [[maybe_unused]] void* after) noexcept
{
	assert(ptr_to_address(before) < ptr_to_address(after) &&
		   "before address is higher than"
		   "after in address space, it can't be before it in the free list");
}

static auto CreateFreeListBlock(uintptr_t address, size_t size) noexcept -> FreeListBlock*
{
	const FreeListBlock data = {.next = nullptr, .size = size};
	void* ptr = address_to_ptr(address);
	std::memcpy(ptr, &data, sizeof(FreeListBlock));
	return static_cast<FreeListBlock*>(ptr);
}

static void WriteFreelistAllocationHeader(void* ptr, size_t size, size_t padding) noexcept
{
	const FreeListAllocationHeader header_data = {.block_size = size, .padding = padding};
	std::memcpy(ptr, &header_data, sizeof(FreeListAllocationHeader));
}

static auto ReadFreelistAllocationHeader(void* ptr) noexcept -> FreeListAllocationHeader
{
	const void* header_ptr = address_to_ptr(ptr_to_address(ptr) - sizeof(FreeListAllocationHeader));
	FreeListAllocationHeader result;
	std::memcpy(&result, header_ptr, sizeof(FreeListAllocationHeader));
	return result;
}

FreeListFirstFitPolicy::FreeListFirstFitPolicy(Block block) noexcept
	: m_baseAddress(block.address)
	, m_size(block.size)
	, m_used(0llu)
	, m_head(CreateFreeListBlock(block.address, block.size))
{
}

static constexpr size_t FreeListMinimumAlignment = 8;
static constexpr size_t FreeListMinimumAllocationSize = sizeof(FreeListBlock);

auto FreeListFirstFitPolicy::allocate(size_t size) noexcept -> Block
{
	return allocate_aligned(size, FreeListMinimumAlignment);
}

auto FreeListFirstFitPolicy::allocate_aligned(size_t size, size_t alignment) noexcept -> Block
{
	if (size < sizeof(FreeListBlock))
	{
		size = sizeof(FreeListBlock);
	}

	if (alignment < FreeListMinimumAlignment)
	{
		alignment = FreeListMinimumAlignment;
	}

	// Iterates the list and finds the first free block with enough space
	FreeListBlock* curr = m_head;
	FreeListBlock* prev = nullptr;

	size_t padding = 0;

	while (curr)
	{
		padding = compute_padding(ptr_to_address(curr), alignment, sizeof(FreeListAllocationHeader));
		const size_t required_space = size + padding;

		if (curr->size >= required_space)
			break;

		prev = curr;
		curr = curr->next;
	}

	if (!curr)
	{
		return NullBlock();
	}

	FreeListBlock* node = curr;
	const size_t alignment_padding = padding - sizeof(FreeListAllocationHeader);
	const size_t required_space = size + padding;
	const size_t remaining = node->size - required_space;

	if (remaining > FreeListMinimumAllocationSize)
	{
		const uintptr_t new_address = ptr_to_address(node) + required_space;
		FreeListBlock* new_node = CreateFreeListBlock(new_address, remaining);

		// insert the new node
		insert_node(node, new_node);
	}

	// unlink the old node from the freelist
	remove_node(prev, node);
	m_used += required_space;

	// Prep the memory we will return to the user
	auto* memory = offset_ptr(node, alignment_padding);
	assert(is_ptr_aligned(memory, alignment) && "ptr isn't aligned correctly");
	WriteFreelistAllocationHeader(memory, required_space, alignment_padding);

	const auto address = ptr_to_address(memory) + sizeof(FreeListAllocationHeader);
	return Block{.address = address, .size = required_space};
}

void FreeListFirstFitPolicy::free(void* ptr) noexcept
{
	if (!ptr)
		return;

	assert(owns_address(ptr_to_address(ptr)) && "ptr not allocated by this allocator");

	const auto header = ReadFreelistAllocationHeader(ptr);
	const auto address = ptr_to_address(ptr) - sizeof(FreeListAllocationHeader) - header.padding;
	FreeListBlock* free_node = CreateFreeListBlock(address, header.block_size);
	FreeListBlock* curr = m_head;
	FreeListBlock* prev = nullptr;

	if (curr == nullptr)
	{
		assert(m_size - m_used <= FreeListMinimumAllocationSize && 
			"allocator isn't empty but m_head is still a nullptr");
		insert_node(curr, free_node);
	}
	else
	{
		bool did_insert = false;
		while (curr != nullptr)
		{
			if (ptr < curr)
			{
				did_insert = true;
				insert_node(prev, free_node);
				break;
			}
			prev = curr;
			curr = curr->next;
		}

		if (prev && curr == nullptr)
		{
			assert(did_insert == false && "Managed to insert a node with curr is a nullptr");
			insert_node(prev, free_node);
		}
	}

	m_used -= free_node->size;

	coalesce(prev, free_node);
}

void FreeListFirstFitPolicy::reset() noexcept
{
	m_head = CreateFreeListBlock(m_baseAddress, m_size);
	m_used = 0llu;
}

auto FreeListFirstFitPolicy::insert_node(FreeListBlock* prev, FreeListBlock* node) noexcept -> void
{
	assert(node && "Trying to insert a nullptr");
	assert(node->next == nullptr && "Node to insert isn't a newly created node");
	assert(ptr_to_address(node) &&
		   "Node is outside the address space controlled"
		   "by this allocator");

	if (m_head == nullptr)
	{
		// If we were able to allocate all the memory this allocator controls, the head
		// of the free list is set to nullptr. When we try to return a block to the
		// free list we end up here
		assert(prev == nullptr && "Head of list is a nullptr but prev isn't");
		m_head = node;
	}
	else if (prev == nullptr)
	{
		// We are returning a block to the free list which has an address in memory before
		// the block currently at the head
		sanity_check_nodes(node, m_head);

		node->next = m_head;
		m_head = node;
	}
	else
	{
		// The most common case where we are just inserting into the free list
		sanity_check_nodes(prev, node);
		assert(ptr_to_address(prev) &&
			   "Prev is outside the address space controlled"
			   "by this allocator");

		node->next = prev->next;
		prev->next = node;
	}
}

auto FreeListFirstFitPolicy::remove_node(FreeListBlock* prev, FreeListBlock* node) noexcept -> void
{
	if (prev == nullptr)
	{
		assert(node == m_head && "prev is a nullptr but we aren't removing the head");
		m_head = node->next;
	}
	else
	{
		prev->next = node->next;
	}
}
auto FreeListFirstFitPolicy::coalesce(FreeListBlock* prev, FreeListBlock* node) noexcept -> void
{
	assert(node && "node is a nullptr");

	if (node->next && offset_ptr(node, node->size) == node->next)
	{
		node->size += node->next->size;
		remove_node(node, node->next);
	}

	if (prev && offset_ptr(prev, prev->size) == node)
	{
		prev->size += node->size;
		remove_node(prev, node);
	}
}

auto FreeListFirstFitPolicy::owns_address(uintptr_t address) const noexcept -> bool
{
	return is_address_in_range(address, m_baseAddress, m_size);
}

} // namespace wmcv