#ifndef WMCV_STACK_ALLOCATOR_H_INCLUDED
#define WMCV_STACK_ALLOCATOR_H_INCLUDED

#include "wmcv_memory_block.h"

namespace wmcv
{
	class StackAllocator
	{
	public:
		StackAllocator(Block block) noexcept;

		[[nodiscard]] auto allocate(size_t size) noexcept -> Block;
		[[nodiscard]] auto allocate_aligned(size_t size, size_t alignment) noexcept -> Block;
		void free(void* ptr) noexcept;
		void reset() noexcept;
	
	private:
		[[nodiscard]] bool owns_address(uintptr_t address) const noexcept;

		uintptr_t m_baseAddress;
		size_t m_size;
		size_t m_previousMarker;
		size_t m_currentMarker;
	};
}

#endif //WMCV_STACK_ALLOCATOR_H_INCLUDED
