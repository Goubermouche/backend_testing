#pragma once
#include <utility/types.h>

namespace baremetal {
	using namespace utility::types;

	class reg {
	public:
		static constexpr u8 invalid_class_index = std::numeric_limits<u8>::max();
		static constexpr u16 invalid_index = std::numeric_limits<u16>::max();

		constexpr reg() : m_class_index(invalid_class_index), m_index(invalid_index) {}
		constexpr reg(u8 class_index, u16 index) : m_class_index(class_index), m_index(index) {}

		[[nodiscard]] constexpr auto get_class_index() const -> u16 {
			return m_class_index;
		}

		[[nodiscard]] constexpr auto get_index() const -> u16 {
			return m_index;
		}

		[[nodiscard]] constexpr auto operator==(reg other) const -> bool {
			return m_class_index == other.get_class_index() && m_index == other.get_class_index();
		}

		[[nodiscard]] constexpr auto is_valid() const -> bool {
			return m_class_index != invalid_class_index && m_index != invalid_index;
		}
	private:
		u8 m_class_index;
		u16 m_index;
	};
} // namespace baremetal
