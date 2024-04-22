#pragma once
#include "baremetal/translation/live_interval.h"
#include "baremetal/translation/instruction.h"
#include "baremetal/translation/work_list.h"
#include "baremetal/pass.h"

#include <utility/memory/memory.h>
#include <utility/managed_ptr.h>

namespace baremetal {
	using namespace utility::types;

	struct transformation_context {
		ptr<ir::function> function;

		// program structure
		std::unordered_map<ptr<ir::node>, ptr<ir::basic_block>> schedule; // node -> parent basic block
		control_flow_graph control_flow_graph;                            // entry node -> basic block
		work_list work_list;                                              // generic node work list
	};

	struct machine_context {
		void append_instruction(ptr<instruction> instruction);
		auto get_virtual_value(ptr<ir::node> node) -> ptr<virtual_value>;
		void use_node(ptr<ir::node> node);

		ptr<ir::function> function;

		// program structure
		std::unordered_map<ptr<ir::node>, ptr<ir::basic_block>> schedule; // node -> parent basic block
		control_flow_graph control_flow_graph;                            // entry node -> basic block
		work_list work_list;                                              // generic node work list

		std::vector<live_interval> intervals;
		std::vector<i32> basic_block_order;                    // map the indices of our work list items to the proper index of our basic blocks

		// node memory/value representations
		std::vector<virtual_value> values;                   // analysis of virtual values and their usage
		std::vector<phi_value> phi_values;                   // conditionally described values

		// instructions
		ptr<instruction> current_instruction;
		ptr<instruction> first_instruction;

		i32 fallthrough_label;
	};

	class target;

	class translation_engine {
	public:
		void translate(target& target, module_data& module);

		void add_pass(managed_ptr<pass> pass);
	private:
		utility::memory<managed_ptr<pass>> m_passes;
	};
} // namespace baremetal
