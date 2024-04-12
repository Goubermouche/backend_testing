#include "control_flow_graph.h"

#include "baremetal/translation/translation_engine.h"
#include "baremetal/dialects/core_dialect.h"

namespace baremetal {
	control_flow_graph::control_flow_graph(ptr<ir::function> function) : m_function(function) {}

	auto control_flow_graph::create_reverse_post_order(transformation_context& context) -> control_flow_graph {
		ASSERT(context.work_list.is_empty(), "invalid work list");
		control_flow_graph cfg(context.function);

		// the entry node is the entry control node
		ptr<ir::block> top = cfg.create_block(context.function->m_parameters[0]);
		context.work_list.is_visited(context.function->m_parameters[0]);

		// visit all blocks, until we return back to the top node (which doesn't have a parent)
		while(top != nullptr) {
			if(top->successor_index > 0) {
				// push the next unvisited successor
				const ptr<ir::node> successor = top->successors[--top->successor_index];

				if(!context.work_list.is_visited(successor)) {
					// create a new block from the current successor
					const ptr<ir::block> new_top = cfg.create_block(successor);
					new_top->parent = top;
					top = new_top;
				}
			}
			else {
				// the current block doesn't have any more control successors, go back and continue
				// exploring the parent block
				ir::block block = *top;

				const ir::basic_block basic_block{
					.dominator_depth = -1,
					.entry = block.basic_block,
					.exit = block.end
				};

				context.work_list.push_back(block.basic_block);
				cfg[block.basic_block] = basic_block;

				// off to wherever we left off
				top = block.parent;
			}
		}

		// reverse our items
		const u64 last = cfg.get_size() - 1;
		for(u64 i = 0; i < cfg.get_size() / 2; ++i) {
			std::swap(context.work_list[i], context.work_list[last - i]);
		}

		// precompute some dominators and mark them with a unique ID
		for(u64 i = 0; i < cfg.get_size(); ++i) {
			const ptr basic_block = &cfg.at(context.work_list[i]);

			if(i == 0) {
				// the primary exit node has a dominator depth of 0
				basic_block->dominator_depth = 0;
			}

			basic_block->id = i;
		}

		return cfg;
	}

	auto control_flow_graph::get_size() const -> u64 {
		return size();
	}

	auto control_flow_graph::get_immediate_dominator(ptr<ir::node> target) -> ptr<ir::node> {
		const auto it = find(target);

		if(it == end()) {
			return nullptr;
		}

		const ptr<ir::basic_block> dominator = it->second.dominator;
		return dominator ? dominator->entry : nullptr;
	}

	auto control_flow_graph::get_traversal_index(ptr<ir::node> target) -> u64 {
		const auto it = find(target);
		if(it == end()) {
			return std::numeric_limits<u64>::max();
		}

		return it->second.id;
	}

	auto control_flow_graph::get_dominator_depth(ptr<ir::node> target) const -> i32 {
		return at(target).dominator_depth;
	}

	auto control_flow_graph::get_control_flow_predecessor(ptr<ir::node> target, u8 index) -> ptr<ir::node> {
		ptr<ir::node> node = target->inputs[index];

		while(true) {
			const auto it = find(node);
			if(it != end() || node->get_node_id() == static_cast<u16>(core_node_id::REGION)) {
				return node;
			}

			node = node->inputs[0];
		}
	}

	auto control_flow_graph::resolve_dominator_depth(ptr<ir::node> basic_block) -> i32 {
		const i32 depth = get_dominator_depth(basic_block);

		// negative depth has a parent we need to visit
		if(depth >= 0) {
			return depth;
		}

		// our depth is just below the parent
		const i32 parent = resolve_dominator_depth(get_immediate_dominator(basic_block));
		at(basic_block).dominator_depth = parent + 1;
		return parent + 1;
	}

	auto control_flow_graph::create_block(ptr<ir::node> basic_block) const -> ptr<ir::node> {
		const ptr<ir::node> end = detail::get_basic_block_end(basic_block);
		u8 successor_count = 1;

		if(end->is_control_flow_endpoint()) {
			successor_count = 0;
		}

		const ptr top = m_function->emplace<ir::block>();
		top->successors = utility::memory_view<ptr<ir::node>>(*m_function, successor_count);
		top->successor_index = successor_count;
		top->basic_block = basic_block;
		top->end = end;

		if(end->is_control_flow_endpoint()) {
			// no successors
		}
		else {
			top->successors[0] = detail::get_next_control_flow_user(end)->node;
		}

		return top;
	}
} // namespace baremetal
