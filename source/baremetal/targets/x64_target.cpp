#include "x64_target.h"

#include "baremetal/dialects/core_dialect.h"
#include "baremetal/dialects/gpu_dialect.h"

namespace baremetal {
	x64_target::x64_target(context& context) : target(context) {
		m_isel_functions.resize(context.m_index_map.size());

		m_isel_functions[context.get_dialect_index<core_dialect>()] = { isel_core };
		m_isel_functions[context.get_dialect_index<gpu_dialect>()]  = { isel_gpu };
	}

	void x64_target::select_instructions(const machine_context& data) {
		std::unordered_set<ptr<ir::node>> visited;

		// for(u64 i = 0; i < data.get_functions().get_size(); ++i) {
		// 	const ptr<ir::function> current = data.get_functions()[i];
		// 
		// 	for(const ptr<ir::node> exit : current->m_terminators) {
		// 		visit_node(exit, visited);
		// 	}
		// 
		// 	visited.clear();
		// }


	}
} // namespace baremetal
