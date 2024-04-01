// block_allocator utility header

#pragma once
#include "utility/allocators/allocator_base.h"
#include "utility/memory/memory.h"

namespace utility {
	/**
	 * \brief Block allocator, a slightly more advanced pool allocator which is capable
	 * of growing. When a block is filled a new one is allocated.
	 */
	class block_allocator : allocator_base {
		struct block {
			block(u8* memory) : memory(memory), position(0) {}
			~block() {
				utility::free(memory);
			}

			u8* memory;
			u64 position;
			block* next = nullptr;
		};
	public:
		/**
		 * \brief Constructs a new block allocator.
		 * \param block_size Size of individual blocks [bytes]
		 */
		block_allocator(u64 block_size) : m_block_size(block_size) {
			allocate_block(); // allocate the first block
			m_first_block = m_current_block;
		}

		virtual ~block_allocator() {
			// free all contained blocks
			while(m_first_block) {
				block* temp = m_first_block;
				m_first_block = m_first_block->next;

				temp->~block();
				utility::free(temp);
			}
		}

		block_allocator(const block_allocator& other) = delete;

		block_allocator(block_allocator&& other) {
			m_first_block = std::move(other.m_first_block);
			m_current_block = std::move(other.m_current_block);

			std::exchange(m_block_size, other.m_block_size);
			std::exchange(m_block_count, other.m_block_count);
		}

		block_allocator& operator=(const block_allocator& other) = delete;

		block_allocator& operator=(block_allocator&& other) {
			if(this != &other) {
				m_first_block = std::move(other.m_first_block);
				m_current_block = std::move(other.m_current_block);

				std::exchange(m_block_size, other.m_block_size);
				std::exchange(m_block_count, other.m_block_count);
			}

			return *this;
		}

		/**
		 * \brief Allocates \b size bytes of uninitialized memory.
		 * \param size Amount of memory to allocate [bytes]
		 * \return Pointer to the beginning of the allocated region.
		 */
		[[nodiscard]] auto allocate(u64 size) -> void* {
			if (size == 0) {
				return nullptr;
			}

			// if this allocation incurs a buffer overflow allocate a new block
			if (m_current_block->position + size >= m_block_size) {
				allocate_block();
			}

			void* memory = m_current_block->memory + m_current_block->position;
			m_current_block->position += size;
			return memory;
		}

		/**
		 * \brief Allocates \b size bytes of zero-initialized memory.
		 * \param size Amount of memory to allocate [bytes]
		 * \return Pointer to the beginning of the allocated region.
		 */
		[[nodiscard]] auto allocate_zero(u64 size) -> void* {
			void* memory = allocate(size);
			std::memset(memory, 0, static_cast<size_t>(size));
			return memory;
		}

		/**
		 * \brief Allocates sizeof(type) bytes of uninitialized memory.
		 * \tparam type Type to allocate memory for
		 * \return Pointer to the allocated object.
		 */
		template<typename type>
		[[nodiscard]] auto allocate() -> type* {
			return static_cast<type*>(allocate(sizeof(type)));
		}

		/**
		 * \brief Allocates sizeof(type) bytes of zero-initialized memory.
		 * \tparam type Type to allocate memory for
		 * \return Pointer to the allocated object.
		 */
		template<typename type>
		[[nodiscard]] auto allocate_zero() -> type* {
			return static_cast<type*>(allocate_zero(sizeof(type)));
		}

		/**
		 * \brief Allocates sizeof(type) bytes of uninitialized memory and constructs \b type.
		 * \tparam type Type of the object to allocate and construct.
		 * \return Pointer to the constructed object.
		 */
		template<typename type, typename... value_types>
		[[nodiscard]] auto emplace(value_types&&... values) -> type* {
			return new (allocate(sizeof(type))) type(std::forward<value_types>(values)...);
		}

		/**
		 * \brief Allocates sizeof(type) bytes of zero-initialized memory and constructs \b type.
		 * \tparam type Type of the object to allocate and construct.
		 * \return Pointer to the constructed object.
		 */
		template<typename type, typename... value_types>
		[[nodiscard]] auto emplace_zero(value_types&&... values) -> type* {
			return new (allocate_zero(sizeof(type))) type(std::forward<value_types>(values)...);
		}

		/**
		 * \brief Retrieves the current amount of allocated blocks.
		 * \return Count of currently allocated blocks.
		 */
		[[nodiscard]] auto get_block_count() const -> u64 {
			return m_block_count;
		}

		/**
		 * \brief Retrieves the max size of individual blocks [bytes].
		 * \return Max size of individual blocks [bytes].
		 */
		[[nodiscard]] auto get_block_size() const -> u64 {
			return m_block_size;
		}
	private:
		/**
		 * \brief Helper function for allocating new memory blocks
		 */
		void allocate_block() {
			const auto memory = static_cast<u8*>(utility::malloc(static_cast<size_t>(m_block_size)));
			const auto new_block = new block(memory);

			if (m_current_block) {
				m_current_block->next = new_block;
			}

			m_current_block = new_block;
			m_block_count++;
		}
	private:
		block* m_first_block = nullptr;
		block* m_current_block = nullptr;

		u64 m_block_size;
		u64 m_block_count = 0;
	};
} // namespace utility
