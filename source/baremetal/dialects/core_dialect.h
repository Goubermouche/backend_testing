#pragma once
#include "baremetal/intermediate_representation/dialect.h"

namespace baremetal {
	struct memory_block {
		u8 alignment;     // bytes
		u8 size;          // bytes
	};

	struct memory_access {
		u8 alignment;
	};

	struct memory_region {
		ptr<ir::node> input_memory;
		ptr<ir::node> output_memory;
	};

	struct projection {
		u8 index;
	};

	enum class core_node : u16 {
		ENTRY,        // function entry node
		EXIT,         // function exit node
		REGION,

		PROJECTION,   // projection for a value
		PHI,          // conditionally defined value

		MEMORY_BLOCK, // generic memory block allocated in the specified address space 
		STORE,        // generic store operation
		LOAD,         // generic load operation

		INTEGER_IMM
	};

	struct integer_immediate {
		u64 value;
	};

	class core_dialect : public dialect {
	public:
		[[nodiscard]] auto get_label(ptr<ir::node> node) const -> std::string_view override;

		void create_function(const ir::function_data_type& data_type);

		[[nodiscard]] auto create_local(u8 alignment, u8 size) -> ptr<ir::node>;
		[[nodiscard]] auto create_signed_integer(i64 value, u8 bit_width) -> ptr<ir::node>;

		void create_store(ptr<ir::node> destination, ptr<ir::node> value, u8 alignment);
		[[nodiscard]] auto create_load(ptr<ir::node> source, ir::data_type type, u8 alignment) -> ptr<ir::node>;
		void create_ret(const utility::memory<ptr<ir::node>, u8>& return_values);

		[[nodiscard]] auto create_projection(ir::data_type type, ptr<ir::node> source, u8 index) -> ptr<ir::node>;

		[[nodiscard]] auto append_memory(ptr<ir::node> new_memory) const->ptr<ir::node>;
		void append_input(ptr<ir::node> target, ptr<ir::node> input);
		[[nodiscard]] static auto get_parent_region(ptr<ir::node> node) -> ptr<ir::node>;
	};

	namespace detail {
		inline auto is_block_start(ptr<ir::node> node) -> bool {
			return
				node->get_node_id() == static_cast<u16>(core_node::REGION) ||
				(node->get_node_id() == static_cast<u16>(core_node::PROJECTION) && node->inputs[0]->get_node_id() == static_cast<u16>(core_node::ENTRY));
		}

		inline auto is_control_flow_control(ptr<ir::node> node) -> bool {
			if(node->get_data_type().get_id() == static_cast<u8>(ir::data_type_id::CONTROL)) { return true; }
			if(node->get_data_type().get_id() == static_cast<u8>(ir::data_type_id::TUPLE)) { return false; }

			return false;
		}

		inline auto get_next_control_flow_user(ptr<ir::node> node) -> ptr<ir::user> {
			for(ptr<ir::user> u = node->users; u; u = u->next) {
				if(is_control_flow_control(u->node)) {
					return u;
				}
			}

			return nullptr;
		}

		inline auto get_predecessor(ptr<ir::node> node, u8 index) -> ptr<ir::node> {
			ptr<ir::node> predecessor = node->inputs[index];

			if(
				node->get_node_id() == static_cast<u16>(core_node::REGION) &&
				predecessor->get_node_id() == static_cast<u16>(core_node::PROJECTION)
				) {
				const ptr<ir::node> parent = predecessor->inputs[0];

				// ENTRY or projections with multiple users
				if(
					parent->get_node_id() == static_cast<u16>(core_node::ENTRY) ||
					(!parent->is_control_projection_node() && predecessor->users->next != nullptr)
					) {
					return predecessor;
				}

				predecessor = parent;
			}

			while(!is_block_start(predecessor)) {
				predecessor = predecessor->inputs[0];
			}

			return predecessor;
		}

		inline auto get_next_control(ptr<ir::node> node, u8 slot) -> ptr<ir::node> {
			for(ptr<ir::user> u = node->users; u; u = u->next) {
				if(u->slot == slot && is_control_flow_control(u->node)) {
					return u->node;
				}
			}

			return nullptr;
		}

		inline auto get_basic_block_end(ptr<ir::node> node) -> ptr<ir::node> {
			while(!node->is_control_flow_terminator()) {
				ptr<ir::node> next = get_next_control(node, 0);

				if(next == nullptr || next->get_node_id() == static_cast<u16>(core_node::REGION)) {
					break;
				}

				node = next;
			}

			return node;
		}
	} // namespace detail
} // namespace baremetal
