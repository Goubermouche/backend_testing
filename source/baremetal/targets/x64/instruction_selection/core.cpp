#include "core.h"
#include "baremetal/targets/x64/x64_target.h"

namespace baremetal::x64 {
	void select_instruction_core(ptr<target> target, ptr<ir::node> node, reg destination) {
		switch(static_cast<core_node_id>(node->get_node_id())) {
			case core_node_id::ENTRY:        select_instruction_core_entry(target, node); break;
			case core_node_id::EXIT:         select_instruction_core_exit(target, node); break;
			case core_node_id::REGION:       break;

			case core_node_id::PROJECTION:   select_instruction_core_projection(target, node, destination); break;
			case core_node_id::PHI:          break;

			case core_node_id::MEMORY_BLOCK: select_instruction_core_memory_block(target, node, destination); break;
			case core_node_id::STORE:        select_instruction_core_store(target, node, destination); break;
			case core_node_id::LOAD:         select_instruction_core_load(target, node, destination); break;

			case core_node_id::INTEGER_IMM:  select_instruction_core_integer(target, node, destination); break;

			default: PANIC("unknown node core node '{}' referenced", static_cast<i32>(node->get_node_id()));
		}
	}

	void select_instruction_core_entry(ptr<x64_target> target, ptr<ir::node> node) {
		// constexpr reg return_registers[4] = { { GPR, RCX }, { GPR, RDX }, { GPR, R8 }, { GPR, R9 }, };
		// const memory_region& entry = node->get_data<memory_region>();
		const ptr<machine_context> context = target->get_context();
		const ptr<instruction> previous = context->current_instruction;

		const ptr<instruction> entry_inst = target->get_assembler().create_instruction(
			static_cast<u16>(instruction_id::ENTRY), ir::I32_TYPE, 0, 0, 0
		);

		target->architecture.address_spaces[STACK].allocate(16, 8);
		entry_inst->next = previous->next;

		utility::console::out("core:entry\n");
	}

	void select_instruction_core_exit(ptr<x64_target> target, ptr<ir::node> node) {
		ASSERT(node->inputs.get_size() <= 5, "at most 2 return values are allowed");

		constexpr reg return_registers[2] = { { GPR, RAX }, { GPR, RDX } };
		const ptr<machine_context> context = target->get_context();
		const assembler& assembler = target->get_assembler();
		const u8 returns = node->inputs.get_size() - 3;

		for(u8 i = 0; i < returns; ++i) {
			const reg source = target->get_virtual_register(node->inputs[3 + i]);
			const ir::data_type dt = node->inputs[3 + i]->get_data_type();

			// copy to the return register
			context->append_instruction(assembler.create_move(dt, return_registers[i], source));
		}

		// we don't really need a fence if we're about to exit but we do need to mark that
		// it's the epilogue to tell the register allocator where callee registers need to
		// get restored
		const ptr<instruction> instruction = assembler.create_instruction(
			static_cast<u16>(instruction_id::EPILOGUE), ir::VOID_TYPE, 0, 0, 0
		);

		instruction->flags |= RETURN;
		context->append_instruction(instruction);

		utility::console::out("core:exit\n");
	}

	void select_instruction_core_projection(ptr<x64_target> target, ptr<ir::node> node, reg destination) {
		const ptr<machine_context> context = target->get_context();
		const assembler& assembler = target->get_assembler();

		if(node->inputs[0]->get_node_id() == static_cast<u16>(core_node_id::ENTRY)) {
			const i32 index = node->get_data<projection>().index - 3;
			constexpr i32 param_gpr_count = 4;

			// past the first register parameters, it's all stack
			if(index >= param_gpr_count) {
				constexpr auto id = instruction_id::MOV;
				context->append_instruction(assembler.create_rm(id, node->get_data_type(), destination, reg{}, 0, 0));
			}
		}

		utility::console::out("core:projection\n");
	}

	void select_instruction_core_memory_block(ptr<x64_target> target, ptr<ir::node> node, reg destination) {
		target->get_context()->append_instruction(select_instruction_core_address(target, node, destination, instruction_id::NONE));
		utility::console::out("core:memory_block\n");
	}

	void select_instruction_core_store(ptr<x64_target> target, ptr<ir::node> node, reg destination) {
		if(destination.is_valid()) {
			target->get_context()->use_node(node->inputs[2]);
			target->get_context()->use_node(node->inputs[3]);
			return;
		}

		const ptr<ir::node> destination_node = node->inputs[2];
		const ptr<ir::node> source_node = node->inputs[3];

		constexpr auto store_op = instruction_id::MOV;

		const u8 bit_count = source_node->get_data_type().get_bit_size();
		i32 immediate;

		if(detail::contains_imm(source_node, immediate, bit_count)) {
			target->get_context()->use_node(source_node);

			const ptr<instruction> store_instruction = select_instruction_core_address(target, destination_node, destination, store_op);
			// store_instruction->in_count -= 1;
			// store_instruction->data_type = legalize_data_type(store_dt);
			store_instruction->flags |= IMMEDIATE;
			// store_instruction->imm = immediate;

			ASSERT(store_instruction->flags & (MEMORY | GLOBAL), "invlaid store flags");
			target->get_context()->append_instruction(store_instruction);
		}
		else {
			const reg source_reg = target->get_virtual_register(source_node);

			const ptr<instruction> store_instruction = select_instruction_core_address(target, destination_node, destination, store_op, source_reg);
			// store_instruction->data_type = legalize_data_type(store_dt);

			ASSERT(store_instruction->flags & (MEMORY | GLOBAL), "invlaid store flags");
			target->get_context()->append_instruction(store_instruction);
		}

		utility::console::out("core:store\n");
	}

	void select_instruction_core_load(ptr<x64_target> target, ptr<ir::node> node, reg destination) {
		constexpr auto mov_op = instruction_id::MOV;
		const ptr<ir::node> source_node = node->inputs[2];

		const ptr<instruction> load_inst = select_instruction_core_address(target, source_node, destination, instruction_id::NONE);
		// load_inst->id = mov_op;
		// load_inst->data_type = legalize_data_type(node->dt);

		target->get_context()->append_instruction(load_inst);
		utility::console::out("core:load\n");
	}

	void select_instruction_core_integer(ptr<x64_target> target, ptr<ir::node> node, reg destination) {
		const integer_immediate& immediate = node->get_data<integer_immediate>();
		u64 value = immediate.value;

		// mask off bits
		if(node->get_data_type().get_size() < 64) {
			value &= (1ull << node->get_data_type().get_size()) - 1;
		}

		if(value == 0) {
			ASSERT(false, "not impl");
		}
		else if((value >> 32ull) == UINT32_MAX) {
			ASSERT(false, "not impl");
		}
		else if(node->get_data_type().get_size() <= 32 || (value >> 31ull) == 0) {
			target->get_context()->append_instruction(target->get_assembler().create_imm(
				instruction_id::MOV, ir::I32_TYPE, destination, static_cast<i32>(value)
			));
		}
		else {
			target->get_context()->append_instruction(target->get_assembler().create_abs(
				instruction_id::MOVABS, node->get_data_type(), destination, value
			));
		}

		utility::console::out("core:imm\n");
	}

	auto select_instruction_core_address(ptr<x64_target> target, ptr<ir::node> node, reg destination, instruction_id id, reg source) -> ptr<instruction> {
		const bool has_second_input = id == instruction_id::NONE && source.is_valid();
		constexpr i32 index = reg::invalid_index;
		i32 offset = 0;
		reg base;

		if(node->get_node_id() == static_cast<u16>(core_node_id::MEMORY_BLOCK)) {
			target->get_context()->use_node(node);
			offset += target->architecture.get_address_slot(STACK, node);
			base = reg(GPR, RBP);
		}
		else {
			base = target->get_virtual_register(node);
		}

		// compute base
		if(id == instruction_id::NONE) {
			if(has_second_input) {
				ASSERT(false, "not implemented");
			}
			else {
				return target->get_assembler().create_rm(instruction_id::LEA, node->get_data_type(), destination, base, index, offset);
			}
		}

		return target->get_assembler().create_mr(id, node->get_data_type(), base, index, offset, source);
	}
} // namespace baremetal::x64
