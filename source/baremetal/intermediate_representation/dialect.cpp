#include "dialect.h"

namespace baremetal {
	auto dialect_base::get_label(u16 id) const -> std::string_view {
		SUPPRESS_C4100(id);
		return "unknown dialect";
	}

	auto dialect_base::get_color(u16 id) const -> std::string_view {
		SUPPRESS_C4100(id);
		return "black";
	}
} // namespace baremetal
