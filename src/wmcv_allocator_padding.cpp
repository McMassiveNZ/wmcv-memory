#include "pch.h"
#include "wmcv_allocator_padding.h"
#include "wmcv_allocator_utility.h"

namespace wmcv
{

size_t compute_padding(uintptr_t ptr, size_t alignment, size_t headerSize) noexcept
{
	assert(is_power_of_two(alignment));

	const uintptr_t remainder = ptr & (alignment - 1);

	uintptr_t padding = ( remainder != 0 ) ? alignment - remainder : 0;
	uintptr_t requiredSpace = headerSize;

	if (padding < requiredSpace)
	{
		requiredSpace -= padding;

		if ((requiredSpace & (alignment - 1)) != 0)
		{
			padding += alignment * (1 + (requiredSpace / alignment));
		}
		else
		{
			padding += alignment * (requiredSpace / alignment);
		}
	}

	return padding;
}

} // namespace wmcv
