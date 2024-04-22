#include "gpu.h"
#include "baremetal/targets/x64/x64_target.h"

namespace baremetal::x64 {
	void select_instruction_gpu(ptr<target> target, ptr<ir::node> node, reg destination) {
		utility::console::out("gpu\n");
	}
} // baremetal::x64
