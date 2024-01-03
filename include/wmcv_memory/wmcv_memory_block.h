#ifndef WMCV_MEMORY_BLOCK_H_INCLUDED
#define WMCV_MEMORY_BLOCK_H_INCLUDED

namespace wmcv
{
struct Block
{
	constexpr bool operator==(Block that) const;
	constexpr bool operator!=(Block that) const;
	constexpr bool operator<(Block that) const;

	uintptr_t address;
	size_t size;
};

constexpr bool Block::operator==(Block that) const
{
	return this->address == that.address && this->size == that.size;
}

constexpr bool Block::operator!=(Block that) const
{
	return !operator==(that);
}

constexpr bool Block::operator<(Block that) const
{
	return this->address < that.address;
}

constexpr Block NullBlock() noexcept
{
	return {.address = uintptr_t{0}, .size = size_t{0}};
}
} // namespace wmcv

#endif // WMCV_MEMORY_BLOCK_H_INCLUDED
