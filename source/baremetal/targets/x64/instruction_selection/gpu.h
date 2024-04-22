#pragma once
#include "baremetal/dialects/gpu_dialect.h"
#include "baremetal/targets/x64/architecture.h"

namespace baremetal {
	class x64_target;

	namespace x64 {
		void select_instruction_gpu(ptr<target> target, ptr<ir::node> node, reg destination);
	} //  namespace x64
} // namespace baremetal
