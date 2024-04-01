#include "node.h"

namespace baremetal::ir {
	auto node_id::operator==(const node_id& other) const -> bool {
		return m_node_id == other.m_node_id && m_dialect == other.m_dialect;
	}

	node::node(u64 global_value_index, node_id id, data_type dt)
		: m_global_value_index(global_value_index), m_id(id), m_data_type(dt), m_data(nullptr) {}

	void node::set_data(void* data) {
		m_data = data;
	}

	auto node::get_global_value_index() const -> u64 {
		return m_global_value_index;
	}

	auto node::get_data_type() const -> data_type {
		return m_data_type;
	}

	auto node::get_id() const -> node_id {
		return m_id;
	}
} // namespace baremetal::ir 
