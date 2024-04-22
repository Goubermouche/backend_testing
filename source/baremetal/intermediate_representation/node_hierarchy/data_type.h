#pragma once
#include <utility/memory/memory_view.h>

namespace baremetal::ir {
	using namespace utility::types;

	enum class data_type_id : u8 {
		UNKNOWN = 0,

		CONTINUATION,
		CONTROL,
		MEMORY,
		TUPLE,

		INTEGER,
		POINTER,
	};

	class data_type {
	public:
		constexpr data_type() : m_id(static_cast<u8>(data_type_id::UNKNOWN)), m_size(0) {}
		constexpr data_type(data_type_id id, u8 size) : m_id(static_cast<u8>(id)), m_size(size) {}
		constexpr data_type(u8 id, u8 size) : m_id(id), m_size(size) {}

		[[nodiscard]] constexpr auto get_id() const -> u8 {
			return m_id;
		}

		[[nodiscard]] constexpr auto get_size() const -> u8 {
			return m_size;
		}

		[[nodiscard]] constexpr auto get_bit_size() const -> u8 {
			if(m_id == static_cast<u8>(data_type_id::POINTER)) {
				return 64;
			}

			return m_size;
		}
	private:
		u8 m_id;   // the underlying type id
		u8 m_size; // get_size of the type in bits
	};

	// core data type constants
	static constexpr data_type CONTINUATION_TYPE = data_type(data_type_id::CONTINUATION, 0);
	static constexpr data_type CONTROL_TYPE = data_type(data_type_id::CONTROL, 0);
	static constexpr data_type MEMORY_TYPE = data_type(data_type_id::MEMORY, 0);
	static constexpr data_type TUPLE_TYPE = data_type(data_type_id::TUPLE, 0);

	static constexpr data_type I8_TYPE  = data_type(data_type_id::INTEGER, 8);
	static constexpr data_type I16_TYPE = data_type(data_type_id::INTEGER, 16);
	static constexpr data_type I32_TYPE = data_type(data_type_id::INTEGER, 32);
	static constexpr data_type I64_TYPE = data_type(data_type_id::INTEGER, 64);

	static constexpr data_type PTR_TYPE = data_type(data_type_id::POINTER, 0);

	static constexpr data_type VOID_TYPE = data_type(data_type_id::INTEGER, 0);

	struct function_data_type {
		utility::memory_view<data_type, u8> parameter_types;
		utility::memory_view<data_type, u8> return_types;

		u8 flags;
	};
} // namespace baremetal::ir
