#include "dialect.h"

namespace baremetal {
	auto dialect_base::get_label(ptr<ir::node> node) const -> std::string_view {
		SUPPRESS_C4100(node);
		return "unknown dialect";
	}

	auto dialect_base::get_color(ptr<ir::node> node) const -> std::string_view {
		SUPPRESS_C4100(node);
		return "black";
	}
} // namespace baremetal
