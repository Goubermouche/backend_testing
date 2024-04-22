#pragma once
#include "baremetal/target.h"

namespace baremetal {
	class x64_target;

	namespace x64 {
		enum address_space : u8 {
			STACK = 0
		};

		enum register_class : u8 {
			GPR = 0,
			XMM
		};

		enum gpr_register : u16 {
			RAX = 0,
			RCX,
			RDX,
			RBX,
			RSI,
			RDI,
			RSP,
			RBP,
			R8,
			R9,
			R10,
			R11,
			R12,
			R13,
			R14,
			R15
		};

		enum xmm_register : u16 {
			XMM0 = 0,
			XMM1,
			XMM2,
			XMM3,
			XMM4,
			XMM5,
			XMM6,
			XMM7,
			XMM8,
			XMM9,
			XMM10,
			XMM11,
			XMM12,
			XMM13,
			XMM14,
			XMM15,
		};

		enum class instruction_id {
			NONE = 0,
			TERMINATOR,
			ENTRY,
			EPILOGUE,

			MOV,
			MOVABS,
			LEA
		};

		void select_instruction_core(ptr<target> target, ptr<ir::node> node, reg destination);

		void select_instruction_core_entry(ptr<x64_target> target, ptr<ir::node> node);
		void select_instruction_core_exit(ptr<x64_target> target, ptr<ir::node> node);
		void select_instruction_core_projection(ptr<x64_target> target, ptr<ir::node> node, reg destination);
		void select_instruction_core_memory_block(ptr<x64_target> target, ptr<ir::node> node, reg destination);
		void select_instruction_core_store(ptr<x64_target> target, ptr<ir::node> node, reg destination);
		void select_instruction_core_load(ptr<x64_target> target, ptr<ir::node> node, reg destination);
		void select_instruction_core_integer(ptr<x64_target> target, ptr<ir::node> node, reg destination);

		auto select_instruction_core_address(ptr<x64_target> target, ptr<ir::node> node, reg destination, instruction_id id, reg source = {}) -> ptr<instruction>;

		void select_instruction_gpu(ptr<target> target, ptr<ir::node> node, reg destination);

		auto create_architecture() -> architecture;

		class assembler : public baremetal::assembler {
		public:
			auto create_label(ptr<ir::node> node) const -> ptr<instruction>;
			auto create_jump(i32 successor) const -> ptr<instruction>;

			auto create_imm(instruction_id kind, ir::data_type dt, reg destination, i32 imm) const -> ptr<instruction>;
			auto create_abs(instruction_id kind, ir::data_type dt, reg destination, u64 imm) const -> ptr<instruction>;

			auto create_move(ir::data_type dt, reg destination, reg source) const -> ptr<instruction>;

			auto create_rm(instruction_id id, ir::data_type dt, reg destination, reg base, i32 index, i32 displacement) const -> ptr<instruction>;
			auto create_mr(instruction_id id, ir::data_type dt, reg base, i32 index, i32 displacement, reg source) const->ptr<instruction>;

			auto create_instruction(u16 instruction_id, ir::data_type data_type, u8 out, u8 in, u8 temp) const -> ptr<instruction>;
		};
	} // namespace x64

	class x64_target : public target {
	public:
		x64_target(context& context);

		void select_instructions(machine_context& context) override;

		auto get_context() const -> ptr<machine_context>;
		auto get_assembler() -> x64::assembler&;

		// registers
		auto get_virtual_register(ptr<ir::node> node) -> reg;
		auto allocate_virtual_register(ptr<ir::node> node, ir::data_type data_type) const->reg;
	private:
		void define_basic_block_order() const;
		void select_instructions();

		void select_region_instructions(ptr<ir::node> block_entry, ptr<ir::node> block_exit, u64 index);

		// utility
		void greedy_schedule(ptr<ir::basic_block> block, ptr<ir::node> block_exit) const;

		auto is_scheduled_in_block(ptr<ir::basic_block> block, ptr<ir::node> node) const -> bool;
		auto create_scheduled_node(ptr<scheduled_node> parent, ptr<ir::node> node) const-> ptr<scheduled_node>;

		static void fill_phi_nodes(std::vector<scheduled_phi>& phi_nodes, ptr<ir::node> successor, i32 phi_index);


	private:
		ptr<machine_context> m_context;
		x64::assembler m_assembler;
	};
} // namespace baremetal
