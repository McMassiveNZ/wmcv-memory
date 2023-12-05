#ifndef WMCV_ALLOCATOR_PADDING_INCLUDED_H
#define WMCV_ALLOCATOR_PADDING_INCLUDED_H

namespace wmcv
{

size_t compute_padding(uintptr_t ptr, size_t alignment, size_t headerSize) noexcept;

}

#endif //WMCV_ALLOCATOR_PADDING_INCLUDED_H