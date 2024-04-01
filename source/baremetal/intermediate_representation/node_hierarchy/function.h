#pragma once
#include "node.h"

#include <utility/memory/memory.h>
#include <utility/allocators/block_allocator.h>

namespace baremetal::ir {
	class function : public utility::block_allocator {
	public:
		function(const function_data_type& data_type);
		~function() override;

		template<typename data_type = std::monostate>
		[[nodiscard]] auto allocate_node(node_id id, u8 input_count, ir::data_type dt) -> ptr<node> {
			const auto new_node = emplace<node>(m_node_count++, id, dt);

			new_node->inputs = utility::memory_view<ptr<node>, u8>(*this, input_count);
			new_node->set_data(emplace<data_type>());

			return new_node;
		}

		utility::memory_view<ptr<node>> m_parameters;
		utility::memory<ptr<node>> m_terminators;

		// function signature
		function_data_type m_data_type;

		ptr<node> m_active_control_node;
		ptr<node> m_entry_node;
		ptr<node> m_exit_node;

		u64 m_node_count;
	};
} // namespace baremetal
