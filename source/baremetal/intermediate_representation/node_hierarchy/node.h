// Nodes
// -   Example node graph:
//     I  J  K
//      \ | /
//       \|/
//        A 
//       / \
//      /   \
//     X     Y
// -   Each node has a list of inputs and users. Given a node 'A', the users of this
//     node are all other nodes which require the value of node 'A' to be evaluated.
// -   In the above graph, node 'A' has 3 inputs {'I', 'J', 'K'} and two users {'X',
//     'Y'}. Note that, nodes 'X' and 'Y' each have one input - {'A'}, in a similar
//     fashion, nodes 'I', 'J', and 'K' have one user {'A'}.

#pragma once
#include "baremetal/intermediate_representation/node_hierarchy/data_type.h"

#include <utility/allocators/block_allocator.h>
#include <utility/macros.h>
#include <utility/ptr.h>

namespace baremetal::ir {
	using namespace utility::types;

	class node_id {
	public:
		constexpr node_id() : m_dialect(0), m_node_id(0) {}
		constexpr node_id(u8 dialect, u16 id) : m_dialect(dialect), m_node_id(id) {}

		auto operator==(const node_id& other) const -> bool;

		[[nodiscard]] constexpr auto get_dialect_id() const -> u8 {
			return m_dialect;
		}

		[[nodiscard]] constexpr auto get_node_id() const -> u16 {
			return m_node_id;
		}
	private:
		u8 m_dialect;
		u16 m_node_id;
	};

	class node;

	struct user {
		ptr<user> next;
		ptr<node> node;
		u8 slot;
	};

	class node {
	public:
		node(u64 global_value_index, node_id id, data_type dt);

		void set_data(void* data);
		void add_user(utility::block_allocator& allocator, ptr<node> input, u8 slot, ptr<user> recycled = nullptr);

		[[nodiscard]] auto get_next_control_flow_user() const -> ptr<user>;
		[[nodiscard]] auto get_predecessor(u8 index) const -> ptr<node>;
		[[nodiscard]] auto get_basic_block_end() const -> ptr<node>;
		[[nodiscard]] auto get_next_control(u8 slot) const -> ptr<node>;

		[[nodiscard]] auto is_control_projection_node() const -> bool;
		[[nodiscard]] auto is_control_flow_terminator() const -> bool;
		[[nodiscard]] auto is_control_flow_endpoint() const -> bool;
		[[nodiscard]] auto is_control_flow_control() const -> bool;
		[[nodiscard]] auto is_block_start() const -> bool;
		[[nodiscard]] auto is_pinned() const -> bool;

		[[nodiscard]] auto get_global_value_index() const -> u64;
		[[nodiscard]] auto get_data_type() const -> data_type;
		[[nodiscard]] auto get_node_id() const -> u16;
		[[nodiscard]] auto get_id() const -> node_id;

		template<typename type>
		[[nodiscard]] auto get_data() -> type& {
			return *static_cast<type*>(m_data);
		}

		utility::memory_view<ptr<node>, u8> inputs; // list of input nodes
		ptr<user> users;
	private:
		u64 m_global_value_index;
		node_id m_id;
		data_type m_data_type;                      // data type of the underlying value
		void* m_data;                               // optional additional data
	};

	struct block {
		ptr<block> parent;
		ptr<node> basic_block;
		ptr<node> end;

		u8 successor_index;
		utility::memory_view<ptr<node>> successors;
	};

	class basic_block {
	public:
		ptr<basic_block> dominator;
		i32 dominator_depth;
		u64 id;

		ptr<node> entry;
		ptr<node> exit;

		ptr<node> memory_input;
		std::unordered_set<ptr<node>> items;
	};
} // namespace baremetal::ir
