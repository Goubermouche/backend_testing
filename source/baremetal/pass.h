#pragma once
#include "baremetal/intermediate_representation/module_data.h"

namespace baremetal {
	struct pass {
		virtual void apply(module_data& data) { SUPPRESS_C4100(data); }
		virtual ~pass() = default;
	};
} // namespace baremetal
