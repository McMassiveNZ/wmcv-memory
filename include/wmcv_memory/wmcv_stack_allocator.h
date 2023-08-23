namespace wmcv
{
	class StackAllocator
	{
	public:
		StackAllocator(std::byte* buffer, size_t size) noexcept;

		[[nodiscard]] auto allocate(size_t size) noexcept -> void*;
		[[nodiscard]] auto allocate_aligned(size_t size, size_t alignment) noexcept -> void*;
		void free(void*) noexcept;
		void reset() noexcept;

		inline size_t internal_get_size() const noexcept { return m_size; }
		inline size_t internal_get_current_marker() const noexcept { return m_currentMarker; }
		inline size_t internal_get_previous_marker() const noexcept { return m_previousMarker; }

	private:
		bool owns_address(uintptr_t address) const noexcept;

		uintptr_t m_baseAddress;
		size_t m_size;
		size_t m_previousMarker;
		size_t m_currentMarker;
	};
}
