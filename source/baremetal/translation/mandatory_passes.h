#pragma once
#include "baremetal/translation/translation_engine.h"

namespace baremetal::detail {
	// use lists
	void generate_use_lists(transformation_context& context);

	// node scheduling
	void schedule_function(transformation_context& context);

	[[nodiscard]] auto find_lca(ptr<ir::basic_block> a, ptr<ir::basic_block> b) -> ptr<ir::basic_block>;

	void schedule_early(transformation_context& context, ptr<ir::node> target);
	void schedule_late(transformation_context& context, ptr<ir::node> target);
} // namespace baremetal::detail
