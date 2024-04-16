#include "gpu_dialect.h"

namespace baremetal {
	auto gpu_dialect::get_label(ptr<ir::node> node) const -> std::string_view {
		SUPPRESS_C4100(node);
		return "gpu test";
	}

	auto gpu_dialect::create_gpu_test(ptr<ir::node> node) -> ptr<ir::node> {
		const ptr<ir::node> test_node = allocate_node(0, 1, ir::data_type{});
		test_node->inputs[0] = node;
		return test_node;
	}
} // namespace baremetal
