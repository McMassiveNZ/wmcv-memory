#ifndef WMCV_ARENA_ALLOCATOR_H_INCLUDED
#define WMCV_ARENA_ALLOCATOR_H_INCLUDED

#include "wmcv_memory_block.h"

namespace wmcv
{
	class ArenaAllocator
	{
	public:
		ArenaAllocator(Block block) noexcept;

		[[nodiscard]] auto allocate(size_t size) noexcept -> Block;
		[[nodiscard]] auto allocate_aligned(size_t size, size_t alignment) noexcept -> Block;

		void free(void*) noexcept;
		void reset() noexcept;

	private:
		uintptr_t m_baseAddress;
		size_t m_size;
		size_t m_marker;
	};
}

#endif //WMCV_ARENA_ALLOCATOR_H_INCLUDED