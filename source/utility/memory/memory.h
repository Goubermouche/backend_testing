// memory utility header

#pragma once
#include "utility/memory/memory_view.h"

namespace utility {
	struct byte {
		byte() = default;
		byte(u8 value) : value(value) {}

		[[nodiscard]] auto to_hex() const -> std::string {
			std::ostringstream oss;
			oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(value);
			return oss.str();
		}

		auto operator |=(const byte& other) -> byte {
			value |= other.value;
			return *this;
		}

		auto operator &=(const byte& other) -> byte {
			value &= other.value;
			return *this;
		}

		auto operator <<(i32 distance) const -> byte {
			return byte(value << distance);
		}

		auto operator >>(i32 distance) const -> byte {
			return byte(value >> distance);
		}

		auto operator +(const byte& other) const->byte {
			return value + other.value;
		}

		operator u8() const {
			return value;
		}

		u8 value;
	};

	[[nodiscard]] inline auto malloc(u64 count) -> void* {
		return std::malloc(static_cast<size_t>(count));
	}

	inline void free(void* ptr) {
		std::free(ptr);
	}

	/**
	 * \brief Aligns the specified \a value into blocks of the specified width.
	 * \param value Value to align
	 * \param alignment Alignment to use
	 * \return Aligned \a value.
	 */
	[[nodiscard]] inline auto align(u64 value, u64 alignment) -> u64 {
		return value + (alignment - (value % alignment)) % alignment;
	}

	[[nodiscard]] inline auto sign_extend(u64 src, u64 src_bits, u64 dst_bits) -> u64 {
		const u64 sign_bit = (src >> (src_bits - 1)) & 1;

		const u64 mask_dst_bits = ~UINT64_C(0) >> (64 - dst_bits);
		const u64 mask_src_bits = ~UINT64_C(0) >> (64 - src_bits);
		const u64 mask = mask_dst_bits & ~mask_src_bits;

		// initialize dst by zeroing out the bits from src_bits to dst_bits
		const u64 dst = src & ~mask;

		// perform the actual sign extension
		return dst | (sign_bit ? mask : 0);
	}

	[[nodiscard]] auto inline fits_into_8_bits(u64 value) -> bool {
		return static_cast<u8>(value) <= std::numeric_limits<u8>::max();
	}

	[[nodiscard]] auto inline fits_into_32_bits(u64 value) -> bool {
		return static_cast<u32>(value) <= std::numeric_limits<u32>::max();
	}

	template<typename type>
	[[nodiscard]] auto pop_count(type value) -> u8 {
		return static_cast<u8>(std::bitset<sizeof(type) * 8>(value).count());
	}

	[[nodiscard]] auto inline ffs(i32 value) -> u8 {
		if(value == 0) {
			return 0;
		}

		u8 pos = 1;
		while(!(value & 1)) {
			value >>= 1;
			pos++;
		}

		return pos;
	}

	[[nodiscard]] auto inline is_power_of_two(u64 value) -> bool {
		return (value & (value - 1)) == 0;
	}

	/**
	 * \brief Owning buffer of memory (basically an std::vector)
	 * \tparam type 
	 */
	template<typename type, typename size_type = u64>
	class memory : public contiguous_memory<type, size_type> {
	public:
		using element_type = type;
		using base_type = contiguous_memory<type, size_type>;

		constexpr memory() : m_capacity(0) {}
		constexpr memory(size_type size) : base_type(allocate(size), size), m_capacity(size) {}
		constexpr memory(size_type size, type* data) : base_type(data, size), m_capacity(size) {}

		constexpr memory(std::initializer_list<type> initializer_list)
		: base_type(allocate(static_cast<size_type>(initializer_list.size())), static_cast<size_type>(initializer_list.size())), m_capacity(this->m_size) {
			std::uninitialized_copy(initializer_list.begin(), initializer_list.end(), this->m_data);
		}

		memory(const memory& other)
		: base_type(nullptr, other.m_size), m_capacity(other.m_capacity) {
			this->m_data = static_cast<type*>(utility::malloc(m_capacity * sizeof(type)));
			std::memcpy(this->m_data, other.m_data, m_capacity * sizeof(type));
		}

		auto operator=(const memory& other) -> memory& {
			if(&other == this) {
				return *this;
			}

			this->m_size = other.m_size;
			m_capacity = other.m_capacity;
			this->m_data = static_cast<type*>(utility::malloc(m_capacity * sizeof(type)));
			std::memcpy(this->m_data, other.m_data, m_capacity * sizeof(type));

			return *this;
		}

		constexpr ~memory() {
			if(!std::is_trivial_v<type>) {
				destruct_range(this->begin(), this->end());
			}

			utility::free(this->m_data);
			this->m_data = nullptr;
		}

		/**
		 * \brief Creates a 0-filled contiguous container with the specified \b size.
		 * \param size Target container size
		 * \return 0-filled contiguous container of the specified \b size.
		 */
		[[nodiscard]] static auto create_zero(size_type size) -> memory {
			const memory container(size);
			container.zero_fill();
			return container;
		}

		/**
		 * \brief Creates a contiguous container with the specified \b capacity
		 * \param capacity Target container capacity
		 * \return Pre-reserved contiguous container.
		 */
		[[nodiscard]] static auto create_reserve(size_type capacity) -> memory {
			memory container;
			container.reserve(capacity);
			return container;
		}

		/**
		 * \brief Resizes the container to 0 and fills all used space with 0's.
		 */
		constexpr void clear() {
			this->m_size = 0;
			// zero_fill();
		}

		/**
		 * \brief 0-fills the entire container.
		 */
		constexpr void zero_fill() const {
			if(this->m_data) {
				std::memset(this->m_data, 0, this->m_size * sizeof(type));
			}
		}

		/**
		 * \brief Appends \b value to the end of the container.
		 * \param value Value to append
		 */
		void push_back(const type& value) {
			if (this->m_size == m_capacity) {
				grow();
			}

			if constexpr (std::is_trivial_v<type>) {
				this->m_data[this->m_size] = value;
			}
			else {
				new (this->m_data + this->m_size) type(value);
			}

			this->m_size++;
		}

		/**
		 * \brief Appends \b value to the end of the container.
		 * \param value Value to append
		 */
		constexpr void push_back(type&& value) {
			if (this->m_size == m_capacity) {
				grow();
			}

			if constexpr (std::is_trivial_v<type>) {
				this->m_data[this->m_size] = value;
			}
			else {
				new (this->m_data + this->m_size) type(std::move(value));
			}

			this->m_size++;
		}

		/**
		 * \brief Emplaces and in-place-constructs an element of the underlying type to the end of the
		 * container.
		 * \tparam arg_types Value constructor types.
		 * \param args Value constructor parameters.
		 */
		template<typename... arg_types>
		constexpr auto emplace_back(arg_types&&... args) -> type& {
			static_assert(
				!std::is_trivial_v<type>, 
				"use push_back() instead of emplace_back() with trivial types"
			);

			if (this->m_size == m_capacity) {
				grow();
			}

			new (this->m_data + this->m_size) type(std::forward<arg_types>(args)...);
			this->m_size++;

			return this->m_data[this->m_size - 1];
		}

		/**
		 * \brief Inserts a range (\b start - \b end) at \b where.
		 * \param where Where to insert
		 * \param start Start of the inserted range
		 * \param end End of the inserted range
		 */
		constexpr void insert(type* where, const type* start, const type* end) {
			const size_type elements_to_insert = end - start;

			if (elements_to_insert == 0) {
				return;
			}

			size_type insert_index = where - this->m_data;

			if (this->m_size + elements_to_insert > m_capacity) {
				reserve(this->m_size + elements_to_insert);

				// update where after reserve as it might have changed
				where = this->m_data + insert_index;
			}

			// move existing elements to make space for new elements
			if (this->m_size > insert_index) {
				if constexpr (std::is_trivially_move_constructible_v<type> && std::is_trivially_destructible_v<type>) {
					std::memmove(where + elements_to_insert, where, (this->m_size - insert_index) * sizeof(type));
				}
				else {
					for (size_type i = this->m_size; i > insert_index; --i) {
						new (this->m_data + i + elements_to_insert - 1) type(std::move(this->m_data[i - 1]));
						this->m_data[i - 1].~type();
					}
				}
			}

			// insert new elements
			for (size_type i = 0; i < elements_to_insert; ++i) {
				if constexpr (std::is_trivially_copyable_v<type>) {
					where[i] = start[i];
				}
				else {
					new (where + i) type(start[i]);
				}
			}

			this->m_size += elements_to_insert;
		}

		/**
		 * \brief Appends a range to the end of the container.
		 * \param source_begin Beginning of the source range
		 * \param source_end End of the source range
		 */
		constexpr void append(const type* source_begin, const type* source_end) {
			insert(this->end(), source_begin, source_end);
		}

		/**
		 * \brief Appends a contiguous container to the end of this container.
		 * \param other Container to append
		 */
		constexpr void append(const memory& other) {
			insert(this->end(), other.begin(), other.end());
		}

		/**
		 * \brief Prepends a contiguous container to the beginning of this container.
		 * \param other Container to prepend
		 */
		constexpr void prepend(const memory& other) {
			insert(this->begin(), other.begin(), other.end());
		}

		/**
		 * \brief Creates a non-owning view/view of this container.
		 * \param start Start of the view
		 * \param size Size of the view
		 * \return view beginning at \b start with the specified \b size.
		 */
		[[nodiscard]] constexpr auto get_view(size_type start, size_type size) const -> memory_view<type> {
			return { this->m_data + start, size };
		}

		/**
		 * \brief Updates the size of the container, \b does \b not \b actually \b change \b the \b memory.
		 * \param new_size New size of the container
		 */
		constexpr void set_size(size_type new_size) {
			this->m_size = new_size;
		}

		/**
		 * \brief Retrieves the current capacity of the container.
		 * \return Capacity of the container
		 */
		[[nodiscard]] constexpr auto get_capacity() const -> size_type {
			return m_capacity;
		}

		/**
		 * \brief Retrieves the first element in the container.
		 * \return First element in the container
		 */
		[[nodiscard]] constexpr auto first() const -> const type& {
			// ASSERT(this->m_data != nullptr && this->m_size > 0, "first() used on an uninitialized container");
			return this->m_data[0];
		}

		/**
		 * \brief Retrieves the first element in the container.
		 * \return First element in the container
		 */
		[[nodiscard]] constexpr auto first() -> type& {
			ASSERT(this->m_data != nullptr && this->m_size > 0, "first() used on an uninitialized container");
			return this->m_data[0];
		}

		/**
		 * \brief Retrieves the last element in the container.
		 * \return Last element in the container
		 */
		[[nodiscard]] constexpr auto last() const -> const type& {
			// ASSERT(this->m_data != nullptr && this->m_size > 0, "last() used on an uninitialized container");
			return this->m_data[this->m_size - 1];
		}

		/**
		 * \brief Retrieves the last element in the container.
		 * \return Last element in the container
		 */
		[[nodiscard]] constexpr auto last() -> type& {
			// ASSERT(this->m_data != nullptr && this->m_size > 0, "last() used on an uninitialized container");
			return this->m_data[this->m_size - 1];
		}

		/**
		 * \brief Reserve enough space for the specified \b capacity.
		 * \param capacity Number of elements to reserve space for
		 */
		constexpr void reserve(size_type capacity) {
			if(m_capacity > capacity) {
				return;
			}

			if constexpr (std::is_trivial_v<type>) {
				this->m_data = static_cast<type*>(std::realloc(this->m_data, sizeof(type) * capacity));
				// ASSERT(this->m_data != nullptr, "reallocation failed");
			}
			else {
				type* new_data = static_cast<type*>(utility::malloc(sizeof(type) * capacity));

				copy_range(this->begin(), this->end(), new_data);
				destruct_range(this->begin(), this->end());

				utility::free(this->m_data);
				this->m_data = new_data;
			}

			m_capacity = capacity;
		}

		/**
		 * \brief Resizes the container to the specified \b size (additional
		 * elements are \b not 0-filled).
		 * \param size Size to resize to
		 */
		constexpr void resize(size_type size) {
			if(this->m_size == size) {
				return;
			}

			if (size > m_capacity) {
				reserve(size);
			}

			if constexpr (!std::is_trivial_v<type>) {
				if (size > this->m_size) {
					construct_range(this->m_data + this->m_size, this->m_data + size);
				}
				else {
					destruct_range(this->m_data + size, this->m_data + this->m_size);
				}
			}

			this->m_size = size;
		}

		[[nodiscard]] constexpr auto pop_back() -> type& {
			return this->m_data[--this->m_size];
		}

		constexpr void pop() {
			--this->m_size;
		}

		/**
		 * \brief Resizes the container to the specified \b size and fills the new
		 * elements with \b value.
		 * \param size Size to resize to
		 * \param value Value to fill the new elements with
		 */
		constexpr void resize(size_type size, const type& value) {
			ASSERT(size != this->m_size, "get_size is already equal to the passed value");

			if (size > m_capacity) {
				reserve(size);
			}

			if constexpr (!std::is_trivial_v<type>) {
				if (size > this->m_size) {
					for (auto p = this->m_data + this->m_size; p != this->m_data + size; ++p) {
						new (p) type(value); // Construct each new element with the given value
					}
				}
				else {
					destruct_range(this->m_data + size, this->m_data + this->m_size);
				}
			}
			else {
				if (size > this->m_size) {
					std::fill(this->m_data + this->m_size, this->m_data + size, value);
				}
			}

			this->m_size = size;
		}
	protected:
		static type* allocate(size_type count) {
			return static_cast<type*>(utility::malloc(count * sizeof(type)));
		}

		void grow() {
			reserve(m_capacity * 2 + 1);
		}

		static constexpr void destruct_range(type* begin, type* end) {
			while (begin != end) {
				begin->~type();
				++begin;
			}
		}

		static constexpr void copy_range(const type* begin, const type* end, type* destination) {
			while (begin != end) {
				new (destination) type(*begin);
				++begin;
				++destination;
			}
		}

		static constexpr void construct_range(type* begin, type* end) {
			while (begin != end) {
				new (begin) type;
				++begin;
			}
		}
	protected:
		size_type m_capacity;
	};
} // namespace utility
