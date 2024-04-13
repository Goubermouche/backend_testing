#include "x64_target.h"

#include "baremetal/dialects/core_dialect.h"
#include "baremetal/dialects/gpu_dialect.h"

namespace baremetal {
	namespace x64 {
		auto create_architecture() -> architecture {
			architecture arch;

			arch.registers.resize(2);
			arch.registers[GPR].resize(16);
			arch.registers[XMM].resize(16);
		 
			for(u16 i = 0; i < 16; ++i) {
				arch.registers[GPR][i] = { GPR, i };
			}

			for(u16 i = 0; i < 16; ++i) {
				arch.registers[XMM][i] = { XMM, i };
			}

			return arch;
		}
	} // namespace x64

	x64_target::x64_target(context& context) : target(context, x64::create_architecture()) {
		m_isel_functions.resize(context.m_index_map.size());

		m_isel_functions[context.get_dialect_index<core_dialect>()] = { x64::isel_core };
		m_isel_functions[context.get_dialect_index<gpu_dialect>()]  = { x64::isel_gpu };
	}

	void x64_target::select_instructions(machine_context& context) {
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

