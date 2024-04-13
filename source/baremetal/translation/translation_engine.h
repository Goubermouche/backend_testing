#pragma once
#include "baremetal/translation/live_interval.h"
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
		ptr<ir::function> function;

		// program structure
		std::unordered_map<ptr<ir::node>, ptr<ir::basic_block>> schedule; // node -> parent basic block
		control_flow_graph control_flow_graph;                            // entry node -> basic block
		work_list work_list;                                              // generic node work list

		std::vector<live_interval> intervals;
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
