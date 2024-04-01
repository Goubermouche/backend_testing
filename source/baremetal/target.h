#pragma once
#include "baremetal/intermediate_representation/module_data.h"
#include "baremetal/dialects/core_dialect.h"

namespace baremetal {
	class target {
	public:
		virtual void select_instructions(const module_data& data) = 0;
	};
} // namespace baremetal
