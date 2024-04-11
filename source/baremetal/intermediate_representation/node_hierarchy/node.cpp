#include "node.h"
#include "baremetal/dialects/core_dialect.h"

namespace baremetal::ir {
	auto node_id::operator==(const node_id& other) const -> bool {
		return m_node_id == other.m_node_id && m_dialect == other.m_dialect;
	}

	node::node(u64 global_value_index, node_id id, data_type dt)
		: m_global_value_index(global_value_index), m_id(id), m_data_type(dt), m_data(nullptr) {}

	void node::set_data(void* data) {
		m_data = data;
	}

	void node::add_user(utility::block_allocator& allocator, ptr<node> input, u8 slot, ptr<user> recycled) {
		const auto new_user = recycled ? recycled : allocator.emplace<user>();

		new_user->next = users;
		new_user->node = input;
		new_user->slot = slot;

		users = new_user;
	}

	auto node::get_next_control_flow_user() const -> ptr<user> {
		for(ptr<user> u = users; u; u = u->next) {
			if(u->node->is_control_flow_control()) {
				return u;
			}
		}

		return nullptr;
	}

	auto node::get_predecessor(u8 index) const -> ptr<node> {
		ptr<node> predecessor = inputs[index];

		if(
			get_node_id() == static_cast<u16>(core_node::REGION) && 
			predecessor->get_node_id() == static_cast<u16>(core_node::PROJECTION)
		) {
			const ptr<node> parent = predecessor->inputs[0];

			// ENTRY or projections with multiple users
			if(
				parent->get_node_id() == static_cast<u16>(core_node::ENTRY) ||
				(!parent->is_control_projection_node() && predecessor->users->next != nullptr)
			) {
				return predecessor;
			}

			predecessor = parent;
		}

		while(!predecessor->is_block_start()) {
			predecessor = predecessor->inputs[0];
		}

		return predecessor;
	}

	auto node::get_basic_block_end() const -> ptr<node> {
		ptr<node> n = (node*)this;

		while(!n->is_control_flow_terminator()) {
			ptr<node> next = n->get_next_control(0);

			if(next == nullptr || next->get_node_id() == static_cast<u16>(core_node::REGION)) {
				break;
			}

			n = next;
		}

		return n;
	}

	auto node::get_next_control(u8 slot) const -> ptr<node> {
		for(ptr<user> u = users; u; u = u->next) {
			if(u->slot == slot && u->node->is_control_flow_control()) {
				return u->node;
			}
		}

		return nullptr;
	}

	auto node::is_control_projection_node() const -> bool {
		return false;
	}

	auto node::is_block_start() const -> bool {
		return
			get_node_id() == static_cast<u16>(core_node::REGION) ||
			(get_node_id() == static_cast<u16>(core_node::PROJECTION) && inputs[0]->get_node_id() == static_cast<u16>(core_node::ENTRY));
	}

	auto node::is_pinned() const -> bool {
		switch(get_node_id()) {
			case static_cast<u16>(core_node::ENTRY):
			case static_cast<u16>(core_node::EXIT):
			case static_cast<u16>(core_node::REGION):
			case static_cast<u16>(core_node::PHI):
			case static_cast<u16>(core_node::PROJECTION): return true;
			default:                                      return false;
		}
	}

	auto node::is_control_flow_terminator() const -> bool {
		switch(get_node_id()) {
			case static_cast<u16>(core_node::EXIT):
				return true;
			default:
				return false;
		}
	}

	auto node::get_global_value_index() const -> u64 {
		return m_global_value_index;
	}

	auto node::is_control_flow_control() const -> bool {
		if(m_data_type.get_id() == static_cast<u8>(data_type_id::CONTROL)) { return true; }
		if(m_data_type.get_id() == static_cast<u8>(data_type_id::TUPLE)) { return false; }

		return false;
	}

	auto node::is_control_flow_endpoint() const -> bool {
		// terminator without successors
		switch(get_node_id()) {
			case static_cast<u16>(core_node::EXIT):
				return true;
			default:
				return false;
		}
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
