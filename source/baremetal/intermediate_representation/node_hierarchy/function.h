#pragma once
#include "node.h"

#include <utility/memory/memory.h>
#include <utility/allocators/block_allocator.h>

namespace baremetal::ir {
	class function : public utility::block_allocator {
	public:
		function(const function_data_type& data_type);
		~function() override;

		/**
		 * \brief Allocates a new node for the given function.
		 * \tparam data_type [optional] Additional data for the node
		 * \param id Id of the new node (dialect id, node id)
		 * \param input_count Number of inputs the node has
		 * \param dt Data type of the node
		 * \return Pointer to the newly allocated node.
		 */
		template<typename data_type = std::monostate>
		[[nodiscard]] auto allocate_node(node_id id, u8 input_count, ir::data_type dt) -> ptr<node> {
			const auto new_node = emplace<node>(m_node_count++, id, dt);

			new_node->inputs = utility::memory_view<ptr<node>, u8>(*this, input_count);
			new_node->set_data(emplace<data_type>());

			return new_node;
		}

		[[nodiscard]] auto get_entry() const->ptr<node>;
		[[nodiscard]] auto get_exit() const -> ptr<node>;
		[[nodiscard]] auto get_active_control() const -> ptr<node>;

		void set_entry(ptr<node> node);
		void set_exit(ptr<node> node);
		void set_active_control(ptr<node> node);

		utility::memory_view<ptr<node>> parameters;
		utility::memory<ptr<node>> terminators;
	private:
		// function signature
		function_data_type m_data_type;

		ptr<node> m_active_control_node;
		ptr<node> m_entry_node;
		ptr<node> m_exit_node;

		u64 m_node_count;
	};
} // namespace baremetal
