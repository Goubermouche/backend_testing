#pragma once
#include "baremetal/target.h"

namespace baremetal {
	inline void isel_core(ptr<ir::node>) {
		std::cout << "core\n";
	}

	inline void isel_gpu(ptr<ir::node>) {
		std::cout << "gpu\n";
	}

	class x64_target : public target {
	public:
		x64_target(context& context);

		void select_instructions(const module_data& data) override {
			std::unordered_set<ptr<ir::node>> visited;

			for(u64 i = 0; i < data.get_functions().get_size(); ++i) {
				const ptr<ir::function> current = data.get_functions()[i];

				for(const ptr<ir::node> exit : current->m_terminators) {
					visit_node(exit, visited);
				}

				visited.clear();
			}
		}

		void visit_node(ptr<ir::node> node, std::unordered_set<ptr<ir::node>>& visited) {
			if(visited.contains(node)) {
				return;
			}

			visited.insert(node);

			for(u8 i = 0; i < node->inputs.get_size(); ++i) {
				if(const ptr<ir::node> input = node->inputs[i]) {
					visit_node(input, visited);
				}
			}

			select_instruction(node);
		}

		void select_instruction(ptr<ir::node> node) {
			m_isel_functions[node->get_id().get_dialect_id()].function(node);
		}
	};


} // namespace baremetal
