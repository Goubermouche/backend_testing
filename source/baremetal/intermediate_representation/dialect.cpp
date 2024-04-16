#include "dialect.h"

namespace baremetal {
	auto dialect_base::get_label(ptr<ir::node> node) const -> std::string_view {
		SUPPRESS_C4100(node);
		return "unknown dialect";
	}
} // namespace baremetal
