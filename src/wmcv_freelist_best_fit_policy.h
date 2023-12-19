#ifndef WMCV_FREELIST_BEST_FIT_POLICY_H_INCLUDED
#define WMCV_FREELIST_BEST_FIT_POLICY_H_INCLUDED

#include "wmcv_memory/wmcv_memory_block.h"
#include "wmcv_freelist_best_fit_policy_detail.h"

namespace wmcv
{
	class FreeListBestFitPolicy
	{
	public:
		FreeListBestFitPolicy(Block block) noexcept;

		[[nodiscard]] auto allocate(size_t size) noexcept -> Block;
		[[nodiscard]] auto allocate_aligned(size_t size, size_t alignment) noexcept -> Block;

		void free(void* ptr) noexcept;
		void reset() noexcept;

		void debug_print() noexcept;

	private:

		using Node = detail::Node;

		auto insert_node(Node* node) noexcept -> void;
		auto remove_node(Node* node) noexcept -> void;
		auto coalesce(Node* node) noexcept -> void;

		[[nodiscard]] auto owns_address(uintptr_t address) const noexcept -> bool;

		uintptr_t m_baseAddress;
		size_t m_size;
        size_t m_used;
    
		Node* m_root;
		Node* m_head;
	};
}

#endif //WMCV_FREELIST_BEST_FIT_POLICY_H_INCLUDED
