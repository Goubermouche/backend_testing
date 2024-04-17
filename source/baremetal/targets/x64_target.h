#pragma once
#include "baremetal/target.h"

namespace baremetal {
	namespace x64 {
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
			TERMINATOR
		};

		inline void isel_core(ptr<ir::node> node, reg destination) {
			SUPPRESS_C4100(node);
			SUPPRESS_C4100(destination);
			std::cout << "core\n";
		}

		inline void isel_gpu(ptr<ir::node> node, reg destination) {
			SUPPRESS_C4100(node);
			SUPPRESS_C4100(destination);
			std::cout << "gpu\n";
		}

		auto create_architecture() -> architecture;

		class assembler : public baremetal::assembler {
		public:
			auto create_label(ptr<ir::node> node) const -> ptr<instruction>;
			auto create_jump(i32 successor) const -> ptr<instruction>;

			auto create_move(ir::data_type dt, reg destination, reg source) const -> ptr<instruction>;

			auto create_instruction(u16 instruction_id, ir::data_type data_type, u8 out, u8 in, u8 temp) const -> ptr<instruction>;
		};
	} // namespace x64

	class x64_target : public target {
	public:
		x64_target(context& context);

		void select_instructions(machine_context& context) override;
	private:
		void define_basic_block_order() const;
		void select_instructions();

		void select_region_instructions(ptr<ir::node> block_entry, ptr<ir::node> block_exit, u64 index);

		// utility
		void greedy_schedule(ptr<ir::basic_block> block, ptr<ir::node> block_exit) const;

		auto is_scheduled_in_block(ptr<ir::basic_block> block, ptr<ir::node> node) const -> bool;
		auto create_scheduled_node(ptr<scheduled_node> parent, ptr<ir::node> node) const-> ptr<scheduled_node>;

		static void fill_phi_nodes(std::vector<scheduled_phi>& phi_nodes, ptr<ir::node> successor, i32 phi_index);

		// registers
		auto get_virtual_register(ptr<ir::node> node) -> reg;
		auto allocate_virtual_register(ptr<ir::node> node, ir::data_type data_type) const -> reg;
	private:
		machine_context* m_context;
		x64::assembler m_assembler;
	};
} // namespace baremetal
