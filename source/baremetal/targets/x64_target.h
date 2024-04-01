#pragma once
#include "baremetal/target.h"

namespace baremetal {
	class x64_target : public target {
	public:
		void select_instructions(const module_data& data) override;
	};
} // namespace baremetal
