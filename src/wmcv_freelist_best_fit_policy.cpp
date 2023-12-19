#include "pch.h"
#include "wmcv_freelist_best_fit_policy.h"
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

static auto CreateFreeListNode(uintptr_t address, size_t size) noexcept -> detail::Node*
{
	const detail::Node data =
	{
		.size = size,
		.color = 0, 
		.children = { detail::Node::SENTINEL(), detail::Node::SENTINEL() },  
		.parent = detail::Node::SENTINEL(),
		.prev = nullptr,
		.next = nullptr
	};

	void* ptr = address_to_ptr(address);
	std::memcpy(ptr, &data, sizeof(detail::Node));
	return static_cast<detail::Node*>(ptr);
}

static void WriteFreelistAllocationHeader(void* ptr, size_t size, size_t padding) noexcept
{
	const FreeListAllocationHeader header_data = {.block_size = size, .padding = padding};
	std::memcpy(ptr, &header_data, sizeof(FreeListAllocationHeader));
}

static auto ReadFreelistAllocationHeader(void* ptr) noexcept -> FreeListAllocationHeader
{
	const void* header_ptr = offset_ptr_back(ptr, sizeof(FreeListAllocationHeader));
	FreeListAllocationHeader result;
	std::memcpy(&result, header_ptr, sizeof(FreeListAllocationHeader));
	return result;
}

FreeListBestFitPolicy::FreeListBestFitPolicy(Block block) noexcept
	: m_baseAddress(block.address)
	, m_size(block.size)
	, m_used(0llu)
	, m_root(CreateFreeListNode(block.address, block.size))
	, m_head(m_root)
{
}

static constexpr size_t FreeListMinimumAlignment = 8;
static constexpr size_t FreeListMinimumAllocationSize = sizeof(detail::Node);

auto FreeListBestFitPolicy::allocate(size_t size) noexcept -> Block
{
	return allocate_aligned(size, FreeListMinimumAlignment);
}

auto FreeListBestFitPolicy::allocate_aligned(size_t size, size_t alignment) noexcept -> Block
{
	assert(m_root && "m_head of the freelist is a nullptr");

	if (size < sizeof(detail::Node))
	{
		size = sizeof(detail::Node);
	}

	if (alignment < FreeListMinimumAlignment)
	{
		alignment = FreeListMinimumAlignment;
	}

	detail::Node* curr = m_root;
	detail::Node* prev = detail::Node::SENTINEL();

	size_t padding = 0;
	size_t required_space = 0;

	while (curr != detail::Node::SENTINEL())
	{
		padding = compute_padding(ptr_to_address(curr), alignment, sizeof(FreeListAllocationHeader));
		required_space = size + padding;

		prev = curr;
		if (required_space == curr->size)
			break;

		if (required_space < curr->size)
			curr = curr->left();
		else
			curr = curr->right();
	}

	while (prev != detail::Node::SENTINEL() && required_space > prev->size)
	{
		prev = prev->parent;
		padding = compute_padding(ptr_to_address(prev), alignment, sizeof(FreeListAllocationHeader));
		required_space = size + padding;
	}

	if ( prev == detail::Node::SENTINEL() )
	{
		return NullBlock();
	}

	detail::Node* node = prev;
	const size_t alignment_padding = padding - sizeof(FreeListAllocationHeader);
	const size_t remaining = node->size - required_space;

	if (remaining > FreeListMinimumAllocationSize)
	{
		const uintptr_t new_address = ptr_to_address(node) + required_space;
		auto* new_node = CreateFreeListNode(new_address, remaining);

		insert_node(new_node);
	}

	remove_node(node);
	m_used += required_space;

	auto* memory = offset_ptr(node, alignment_padding);
	assert(is_ptr_aligned(memory, alignment) && "ptr isn't aligned correctly");
	WriteFreelistAllocationHeader(memory, required_space, alignment_padding);

	const auto address = ptr_to_address(memory) + sizeof(FreeListAllocationHeader);
	return Block{.address = address, .size = required_space};
}

void FreeListBestFitPolicy::free(void* ptr) noexcept
{
	if (!ptr)
		return;

	assert(owns_address(ptr_to_address(ptr)) && "ptr not allocated by this allocator");

	const auto header = ReadFreelistAllocationHeader(ptr);
	const auto address = ptr_to_address(ptr) - sizeof(FreeListAllocationHeader) - header.padding;
	auto* node = CreateFreeListNode(address, header.block_size);

	insert_node(node);

	m_used -= node->size;

	coalesce(node);
}

void FreeListBestFitPolicy::reset() noexcept
{
	m_root = CreateFreeListNode(m_baseAddress, m_size);
	m_head = m_root;
	m_used = 0llu;
}

void FreeListBestFitPolicy::debug_print() noexcept
{
	DebugPrint(m_root);
}

auto FreeListBestFitPolicy::insert_node(detail::Node* node) noexcept -> void
{
	Insert(m_root, node);
	ListInsert(m_head, node);
}

auto FreeListBestFitPolicy::remove_node(detail::Node* node) noexcept -> void
{
	Remove(m_root, node);
	ListRemove(m_head, node);
}

auto FreeListBestFitPolicy::coalesce(detail::Node* node) noexcept -> void
{
	assert(node && "node is a nullptr");

	if (node->next && offset_ptr(node, node->size) == node->next)
	{
		node->size += node->next->size;
		remove_node(node->next);
	}

	if (node->prev && offset_ptr(node->prev, node->prev->size) == node)
	{
		node->prev->size += node->size;
		remove_node(node);
	}
}

auto FreeListBestFitPolicy::owns_address(uintptr_t address) const noexcept -> bool
{
	return is_address_in_range(address, m_baseAddress, m_size);
}

} // namespace wmcv
