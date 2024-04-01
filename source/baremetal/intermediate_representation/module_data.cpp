#include "module_data.h"
#include "dialect.h"

namespace baremetal {
	module_data::module_data() : block_allocator(1024) {}

	module_data::~module_data() {
		for(const ptr<ir::function> function : m_functions) {
			function->~function();
		}
	}

	auto module_data::get_functions() const -> const utility::memory<ptr<ir::function>>& {
		return m_functions;
	}

	auto module_data::get_dialect(u64 id) const -> dialect_base* {
		return m_dialect_pointers.at(id);
	}

	void module_data::allocate_function(const ir::function_data_type& data_type) {
		m_functions.push_back(emplace<ir::function>(data_type));
	}
} // namespace baremetal
