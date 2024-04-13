#pragma once
#include "baremetal/intermediate_representation/node_hierarchy/function.h"

namespace baremetal {
	using namespace utility::types;

	struct transformation_context;

	class control_flow_graph : public std::unordered_map<ptr<ir::node>, ir::basic_block> {
		using base_type = std::unordered_map<ptr<ir::node>, ir::basic_block>;

		control_flow_graph(ptr<ir::function> function);
	public:
		static auto create_reverse_post_order(transformation_context& context) -> control_flow_graph;
		control_flow_graph() = default;

		[[nodiscard]] auto get_size() const -> u64;

		[[nodiscard]] auto get_immediate_dominator(ptr<ir::node> target) -> ptr<ir::node>;
		[[nodiscard]] auto get_traversal_index(ptr<ir::node> target) -> u64;
		[[nodiscard]] auto get_dominator_depth(ptr<ir::node> target) const->i32;
		[[nodiscard]] auto get_control_flow_predecessor(ptr<ir::node> target, u8 index) -> ptr<ir::node>;

		auto resolve_dominator_depth(ptr<ir::node> basic_block) -> i32;
	private:
		auto create_block(ptr<ir::node> basic_block) const -> ptr<ir::node>;
	private:
		ptr<ir::function> m_function;
	};
} // namespace baremetal
