#include "target.h"

namespace baremetal {
	target::target(context& context) : m_context(context) {}

	auto target::get_context() const -> context& {
		return m_context;
	}
} // namespace baremetal
