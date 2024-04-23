#include "instruction.h"

namespace baremetal {
	instruction::instruction()
		: flags(NONE), next(nullptr), m_out_count(0), m_in_count(0), m_temp_count(0) {}

	instruction::instruction(u8 out_count, u8 in_count, u8 temp_count)
		: flags(NONE), next(nullptr), m_out_count(out_count), m_in_count(in_count), m_temp_count(temp_count) {}

	auto instruction::get_out_count() const -> u8 {
		return m_out_count;
	}

	auto instruction::get_in_count() const -> u8 {
		return m_in_count;
	}

	auto instruction::get_temp_count() const -> u8 {
		return m_temp_count;
	}

	auto instruction::get_operand_count() const -> u8 {
		return get_out_count() + get_in_count() + get_temp_count();
	}
} // namespace baremetal
