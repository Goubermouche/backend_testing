#include "function.h"

namespace baremetal::ir {
	function::function(const function_data_type& data_type)
		: block_allocator(1024), m_data_type(data_type), m_node_count(0) {}

	function::~function() {}

	auto function::get_entry() const -> ptr<node> {
		return m_entry_node;
	}

	auto function::get_exit() const -> ptr<node> {
		return m_exit_node;
	}

	auto function::get_active_control() const -> ptr<node> {
		return m_active_control_node;
	}

	auto function::get_node_count() const -> u64 {
		return m_node_count;
	}

	void function::set_entry(ptr<node> node) {
		m_entry_node = node;
	}

	void function::set_exit(ptr<node> node) {
		m_exit_node = node;
	}

	void function::set_active_control(ptr<node> node) {
		m_active_control_node = node;
	}
} // namespace baremetal::ir 
