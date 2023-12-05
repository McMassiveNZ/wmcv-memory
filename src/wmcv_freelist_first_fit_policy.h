#ifndef WMCV_FREELIST_FIRST_FIT_POLICY_H_INCLUDED
#define WMCV_FREELIST_FIRST_FIT_POLICY_H_INCLUDED

#include "wmcv_memory/wmcv_memory_block.h"

namespace wmcv
{
    struct FreeListBlock 
    {
        FreeListBlock *next;
        size_t size;
    };

	class FreeListFirstFitPolicy
	{
	public:
		FreeListFirstFitPolicy(Block block) noexcept;

		[[nodiscard]] auto allocate(size_t size) noexcept -> Block;
		[[nodiscard]] auto allocate_aligned(size_t size, size_t alignment) noexcept -> Block;

		void free(void* ptr) noexcept;
		void reset() noexcept;

	private:

		auto insert_node(FreeListBlock* prev, FreeListBlock* node) noexcept -> void;
		auto remove_node(FreeListBlock* prev, FreeListBlock* node) noexcept -> void;
		auto coalesce(FreeListBlock* prev, FreeListBlock* node) noexcept -> void;

		[[nodiscard]] auto owns_address(uintptr_t address) const noexcept -> bool;

		uintptr_t m_baseAddress;
		size_t m_size;
        size_t m_used;
    
        FreeListBlock* m_head;
	};
}

#endif //WMCV_FREELIST_FIRST_FIT_POLICY_H_INCLUDED