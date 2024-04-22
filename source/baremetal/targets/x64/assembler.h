#pragma once
#include "baremetal/targets/x64/architecture.h"

namespace baremetal::x64 {
	class assembler : public baremetal::assembler {
	public:
		auto create_label(ptr<ir::node> node) const->ptr<instruction>;
		auto create_jump(i32 successor) const->ptr<instruction>;

		auto create_imm(instruction_id kind, ir::data_type dt, reg destination, i32 imm) const->ptr<instruction>;
		auto create_abs(instruction_id kind, ir::data_type dt, reg destination, u64 imm) const->ptr<instruction>;

		auto create_move(ir::data_type dt, reg destination, reg source) const->ptr<instruction>;

		auto create_rm(instruction_id id, ir::data_type dt, reg destination, reg base, i32 index, i32 displacement) const->ptr<instruction>;
		auto create_mr(instruction_id id, ir::data_type dt, reg base, i32 index, i32 displacement, reg source) const->ptr<instruction>;

		auto create_instruction(u16 instruction_id, ir::data_type data_type, u8 out, u8 in, u8 temp) const->ptr<instruction>;
	};
} // namespace baremetal::x64
