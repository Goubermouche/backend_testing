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

		void select_instructions(const machine_context& data) override;

		void select_instruction(ptr<ir::node> node) {
			m_isel_functions[node->get_id().get_dialect_id()].function(node);
		}
	};
} // namespace baremetal
