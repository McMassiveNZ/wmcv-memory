#ifndef WMCV_FREE_LIST_ALLOCATOR_H_INCLUDED
#define WMCV_FREE_LIST_ALLOCATOR_H_INCLUDED

#include "wmcv_memory_block.h"

namespace wmcv
{
	template<typename T>
	concept FreeListPolicy = requires(T t) {
		t.allocate(size_t{});
		t.allocate_aligned(size_t{}, size_t{});
		t.free(nullptr);
		t.reset();
	};
	
	template< FreeListPolicy Policy >
	class FreeListAllocator
	{
	public:
		FreeListAllocator(Block block) noexcept
			: m_policy(block)
		{
		}

		[[nodiscard]] auto allocate(size_t size) noexcept -> Block 
		{
			m_policy.allocate(size);
		}

		[[nodiscard]] auto allocate_aligned(size_t size, size_t alignment) noexcept -> Block
		{
			m_policy.allocate_aligned(size, alignment);
		}

		void free(void* ptr) noexcept
		{
			m_policy.free(ptr);
		}

		void reset() noexcept
		{
			m_policy.reset();
		}

	private:
		Policy m_policy;
	};
}

#endif //WMCV_FREE_LIST_ALLOCATOR_H_INCLUDED