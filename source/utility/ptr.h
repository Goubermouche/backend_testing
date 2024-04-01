// ptr utility header

#pragma once
#include "utility/types.h"

namespace utility::types {
	// NOTE: due to the lightweight nature of this class, atomic operations are NOT used
	//       and thus the container is not thread-safe. 

	/**
	 * \brief Simple and lightweight non-owning pointer abstraction.
	 * \tparam type Type of the contained pointer
	 */
	template<typename type>
	class ptr {
	public:
		ptr() = default;
		ptr(type* ptr) : m_ptr(ptr) {}

		template<typename other_type>
		ptr(other_type* other) : m_ptr(reinterpret_cast<type*>(other)) {}

		template<typename other_type>
		ptr(ptr<other_type> other) : m_ptr(reinterpret_cast<type*>(other.get())) {}

		auto operator*() const -> type& {
			return *m_ptr;
		}

		auto operator->() const -> type* {
			return m_ptr;
		}

		[[nodiscard]] auto get() const -> type* {
			return m_ptr;
		}

		auto operator==(const ptr& other) const -> bool {
			return m_ptr == other.m_ptr;
		}

		operator bool() const noexcept {
			return m_ptr != nullptr;
		}
	protected:
		type* m_ptr = nullptr;
	};
} // namespace utility

template<typename type>
struct std::hash<utility::types::ptr<type>> {
	utility::u64 operator()(const utility::types::ptr<type>& h) const noexcept {
		// hash the internal pointer
		return std::hash<type*>{}(h.get());
	}
};

template<typename type>
struct std::formatter<utility::types::ptr<type>> : std::formatter<std::string> {
	auto format(utility::types::ptr<type> obj, format_context& ctx) const {
		if (obj) {
			return format_to(ctx.out(), "{}", reinterpret_cast<utility::u64>(obj.get()));
		}

		return format_to(ctx.out(), "0");
	}
};
