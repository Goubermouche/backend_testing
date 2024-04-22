#pragma once
#include "baremetal/targets/x64/instruction_selection/core.h"
#include "baremetal/targets/x64/instruction_selection/gpu.h"
#include "baremetal/targets/x64/assembler.h"

namespace baremetal {
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
