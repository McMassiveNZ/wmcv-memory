#ifndef WMCV_ALLOCATOR_UTILITY_H_INCLUDED
#define WMCV_ALLOCATOR_UTILITY_H_INCLUDED

namespace wmcv
{

[[nodiscard]] inline auto ptr_to_address(void* ptr) noexcept -> uintptr_t
{
	return reinterpret_cast<uintptr_t>(ptr);
}

[[nodiscard]] inline auto address_to_ptr(uintptr_t address) noexcept -> void*
{
	return reinterpret_cast<void*>(address);
}

[[nodiscard]] constexpr auto is_power_of_two(size_t n) -> bool
{
	const bool result = (n & (n - 1)) == 0;
	return result;
}

[[nodiscard]] constexpr auto align(uintptr_t address, size_t alignment) -> uintptr_t
{
	assert(is_power_of_two(alignment) && "Error: alignment isn't a power of 2");
	const uintptr_t mask = alignment - 1;
	const uintptr_t result = ( address + mask ) & ~mask;
	return result;
}

[[nodiscard]] constexpr auto is_aligned(size_t size, size_t alignment) -> bool
{
	assert(is_power_of_two(alignment) && "Error: alignment isn't a power of 2");
	const bool result = (size & (alignment - 1)) == 0;
	return result;
}

[[nodiscard]] inline auto is_ptr_aligned(void* ptr, size_t alignment) -> bool
{
	assert(is_power_of_two(alignment) && "Error: alignment isn't a power of 2");
	const uintptr_t uintptr = ptr_to_address(ptr);
	const bool result = is_aligned(uintptr, alignment);
	return result;
}

constexpr void zero_memory(void* ptr, size_t size) noexcept
{
	const std::span<std::byte> block(static_cast<std::byte*>(ptr), size);
	std::fill(block.begin(), block.end(), std::byte{0});
}

} // namespace wmcv

[[nodiscard]] constexpr auto operator""_kB(unsigned long long size) -> unsigned long long
{
	return size * 1024;
}

[[nodiscard]] constexpr auto operator""_MB(unsigned long long size) -> unsigned long long
{
	return size * 1024 * 1024;
}

[[nodiscard]] constexpr auto operator""_GB(unsigned long long size) noexcept -> unsigned long long
{
	return size * 1024 * 1024 * 1024;
}

#endif //WMCV_ALLOCATOR_UTILITY_H_INCLUDED