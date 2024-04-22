#include "node.h"

namespace baremetal::ir {
	auto node_id::operator==(const node_id& other) const -> bool {
		return m_node_id == other.m_node_id && m_dialect == other.m_dialect;
	}

	node::node(u64 global_value_index, node_id id, data_type dt)
		: flags(NONE), m_global_value_index(global_value_index), m_id(id), m_data_type(dt), m_data(nullptr) {}

	void node::set_data(void* data) {
		m_data = data;
	}

	void node::add_user(utility::block_allocator& allocator, ptr<node> input, u8 slot, ptr<user> recycled) {
		const auto new_user = recycled ? recycled : allocator.emplace<user>();

		new_user->next = users;
		new_user->target = input;
		new_user->slot = slot;

		users = new_user;
	}
	
	auto node::is_control_projection_node() const -> bool {
		return flags & IS_CONTROL_PROJECTION;
	}
	
	auto node::is_pinned() const -> bool {
		return flags & IS_PINNED;
	}

	auto node::is_control_flow_terminator() const -> bool {
		return flags & IS_CONTROL_FLOW_TERMINATOR;
	}

	auto node::is_control_flow_endpoint() const -> bool {
		// terminator without successors
		return flags & IS_CONTROL_FLOW_ENDPOINT;
	}

	auto node::is_memory_out_operator() const -> bool {
		return m_data_type.get_id() == static_cast<u8>(data_type_id::MEMORY) || flags & IS_MEMORY_OUT_OPERATOR;
	}

	auto node::get_global_value_index() const -> u64 {
		return m_global_value_index;
	}

	auto node::get_data_type() const -> data_type {
		return m_data_type;
	}

	auto node::get_node_id() const -> u16 {
		return m_id.get_node_id();
	}

	auto node::get_id() const -> node_id {
		return m_id;
	}
} // namespace baremetal::ir 
