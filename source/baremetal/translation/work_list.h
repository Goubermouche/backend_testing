#pragma once
#include "baremetal/intermediate_representation/node_hierarchy/function.h"
#include "baremetal/translation/control_flow_graph.h"

namespace baremetal {
	using namespace utility::types;

	class work_list : public utility::memory<ptr<ir::node>> {
		using base_type = memory;
	public:
		void push_all(ptr<ir::function> function);
		void compute_dominators(control_flow_graph& cfg);

		auto is_visited(ptr<ir::node> node) -> bool;

		std::unordered_set<ptr<ir::node>> visited;
	};
} // namespace baremetal
