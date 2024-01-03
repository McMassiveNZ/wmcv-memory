#ifndef WMCV_LOCKLESS_ARENA_ALLOCATOR_H_INCLUDED
#define WMCV_LOCKLESS_ARENA_ALLOCATOR_H_INCLUDED

#include "wmcv_memory_block.h"

namespace wmcv
{
class LocklessArenaAllocator
{
public:
	LocklessArenaAllocator(Block block) noexcept;

	[[nodiscard]] auto allocate(size_t size) noexcept -> Block;
	[[nodiscard]] auto allocate_aligned(size_t size, size_t alignment) noexcept -> Block;

	void free(void*) noexcept;
	void reset() noexcept;

private:
	uintptr_t m_baseAddress;
	size_t m_size;
	std::atomic_size_t m_marker;
};
} // namespace wmcv

#endif // WMCV_LOCKLESS_ARENA_ALLOCATOR_H_INCLUDED