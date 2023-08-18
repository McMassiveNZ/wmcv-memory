#pragma once

namespace wmcv
{
	class ArenaAllocator
	{
	public:
		ArenaAllocator(std::byte* buffer, size_t size) noexcept;

		[[nodiscard]] auto allocate(size_t size) noexcept -> void*;
		[[nodiscard]] auto allocate_aligned(size_t size, size_t alignment) noexcept -> void*;

		void free(void*) noexcept;
		void reset() noexcept;

		inline size_t internal_get_size() const noexcept { return m_size; }
		inline size_t internal_get_marker() const noexcept { return m_marker; }

	private:
		std::byte* m_memory;
		size_t m_size;
		size_t m_marker;
	};
}