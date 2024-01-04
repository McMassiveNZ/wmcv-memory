#include "pch.h"

#include "wmcv_lockless_arena_allocator.h"
#include "wmcv_allocator_utility.h"

namespace wmcv
{

LocklessArenaAllocator::LocklessArenaAllocator(Block block) noexcept
: m_baseAddress(block.address)
, m_size(block.size)
, m_marker(0llu)
{
}

auto LocklessArenaAllocator::allocate(size_t size) noexcept -> Block
{
    constexpr size_t s_default_alignment = 16;
    return allocate_aligned(size, s_default_alignment);
}

auto LocklessArenaAllocator::allocate_aligned(size_t size, size_t alignment) noexcept -> Block
{
    size_t old_marker = m_marker.load();
    size_t new_marker = 0;
    
    do
    {
        const uintptr_t curr_ptr = m_baseAddress + old_marker;
        new_marker = align(curr_ptr, alignment) + size - m_baseAddress;
        
        if (new_marker > m_size)
            return NullBlock();
        
    } while( !m_marker.compare_exchange_weak(old_marker, new_marker) );
    
    const uintptr_t offset = new_marker - size;
    const uintptr_t address = m_baseAddress + offset;
    return { .address = address, .size = size };
}

void LocklessArenaAllocator::free(void*) noexcept
{
}

void LocklessArenaAllocator::reset() noexcept
{
    m_marker.store(0llu);
}

}
