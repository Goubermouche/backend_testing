// handle utility header

#pragma once
#include "utility/allocators/allocator_base.h"
#include "utility/ptr.h"

namespace utility::types {
	/**
	 * \brief Higher-level handle. Used as an abstraction for objects allocated via an allocator. Once
	 * the lifetime of the shared handle container and all of it's references reaches its end the contained
	 * object's destructor gets called. 
	 * \tparam type 
	 * \tparam count_type 
	 */
	template<typename type, typename count_type = u64>
	class managed_ptr {
	public:
		managed_ptr() = default;

		managed_ptr(const managed_ptr& other)
			: m_ptr(other.m_ptr), m_count(other.m_count) {
			if (m_count) {
				(*m_count)++;
			}
		}

		template<typename other_type>
		managed_ptr(const managed_ptr<other_type>& other)
			: m_ptr(reinterpret_cast<type*>(other.get())), m_count(other.get_count()) {
			if (m_count) {
				(*m_count)++;
			}
		}

		template<typename allocator, typename... arg_types>
			requires is_allocator<allocator>
		managed_ptr(allocator& alloc, arg_types&&... args)
			: m_ptr(alloc.template emplace<type>(std::forward<arg_types>(args)...)),
			m_count(alloc.template emplace<count_type>(1)) {}

		auto operator=(const managed_ptr& other) -> managed_ptr& {
			if (this != &other) {
				release();
				m_ptr = other.m_ptr;
				m_count = other.m_count;
				if (m_count) {
					(*m_count)++;
				}
			}
			return *this;
		}

		template<typename other_type>
		auto operator=(managed_ptr<other_type>& other) -> managed_ptr& {
			if(reinterpret_cast<void*>(this) != reinterpret_cast<void*>(&other)) {
				release();
				m_ptr = reinterpret_cast<type*>(other.get());
				m_count = other.get_count();
				if (m_count) {
					(*m_count)++;
				}
			}

			return *this;
		}

		~managed_ptr() {
			release();
		}

		type* operator->() const {
			return m_ptr;
		}

		type* get() const {
			return m_ptr;
		}

		count_type get_use_count() const {
			return (m_count ? *m_count : 0);
		}

		count_type* get_count() const {
			return m_count;
		}

	private:
		void release() {
			if (m_count && --(*m_count) == 0) {
				m_ptr->~type();
				m_count->~count_type();
			}
		}

		type* m_ptr = nullptr;
		count_type* m_count = nullptr;
	};
} // namespace utility
