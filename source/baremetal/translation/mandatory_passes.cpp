#include "mandatory_passes.h"
#include "baremetal/dialects/core_dialect.h"

namespace baremetal::detail {
	void generate_use_lists(transformation_context& context) {
		ASSERT(context.work_list.is_empty(), "invalid work list");

		// collect all nodes in the function
		context.work_list.push_all(context.function);

		for(const ptr<ir::node> item : context.work_list) {
			// mark the item as a user of all its input nodes
			for(u8 slot = 0; slot < item->inputs.get_size(); ++slot) {
				if(const ptr<ir::node> input = item->inputs[slot]) {
					input->add_user(*context.function, item, slot);
				}
			}
		}

		context.work_list.clear();
	}

	void schedule_function(transformation_context& context) {
		constexpr ir::node_id projection_id(0, static_cast<u16>(core_node_id::PROJECTION));
		constexpr ir::node_id phi_id(0, static_cast<u16>(core_node_id::PHI));

		// generate graph dominators
		context.control_flow_graph = control_flow_graph::create_reverse_post_order(context);
		context.work_list.compute_dominators(context.control_flow_graph);
		context.work_list.visited.clear();
		context.schedule.reserve(256);

		for(u64 i = 0; i < context.control_flow_graph.get_size(); ++i) {
			const auto best = &context.control_flow_graph.at(context.work_list[i]);
			best->items.reserve(32);
		}

		// pinned scheduling
		for(u64 i = context.control_flow_graph.get_size(); i-- > 0;) {
			const ptr<ir::node> basic_block_entry = context.work_list[i];
			const ptr<ir::basic_block> basic_block = &context.control_flow_graph.at(basic_block_entry);
			ptr<ir::node> current = basic_block->exit;

			if(i == 0) {
				// schedule the ENTRY node
				ptr<ir::node> entry = context.function->get_entry();
				basic_block->items.insert(entry);
				context.schedule[entry] = basic_block;
			}

			while(true) {
				basic_block->items.insert(current);
				context.schedule[current] = basic_block;

				// projections should go into the same block
				for(ptr<ir::user> user = current->users; user; user = user->next) {
					const ptr<ir::node> projection = user->node; // potential projection

					if(
						user->slot == 0 &&
						(projection->get_id() == projection_id || projection->get_id() == phi_id)
						) {
						if(!context.schedule.contains(projection)) {
							basic_block->items.insert(projection);
							context.schedule[projection] = basic_block;
						}
					}
				}

				if(current == basic_block_entry) {
					break;
				}

				current = current->inputs[0];
			}
		}

		// early schedule
		for(u64 i = context.control_flow_graph.get_size(); i-- > 0;) {
			schedule_early(context, context.control_flow_graph.at(context.work_list[i]).exit);
		}

		// move nodes closer to their usage site
		for(const ptr<ir::node> item : context.work_list) {
			schedule_late(context, item);
		}

		context.work_list.set_size(context.control_flow_graph.get_size());
		context.work_list.visited.clear();
	}

	auto find_lca(ptr<ir::basic_block> a, ptr<ir::basic_block> b) -> ptr<ir::basic_block> {
		if(a == nullptr) {
			return b;
		}

		// line both up
		while(a->dominator_depth > b->dominator_depth) { a = a->dominator; }
		while(b->dominator_depth > a->dominator_depth) { b = b->dominator; }

		while(a != b) {
			b = b->dominator;
			a = a->dominator;
		}

		return a;
	}

	void schedule_early(transformation_context& context, ptr<ir::node> target) {
		if(context.work_list.is_visited(target)) {
			return;
		}

		// push our target, late scheduling will process this list
		context.work_list.push_back(target);

		// schedule inputs first
		for(const ptr<ir::node> input : target->inputs) {
			if(input) {
				schedule_early(context, input);
			}
		}

		// skip pinned nodes
		if(target->is_pinned() && !target->inputs.is_empty()) {
			return;
		}

		// start at the entry node
		ptr<ir::basic_block> best = context.schedule.at(context.work_list[0]);
		i32 best_depth = 0;

		// choose the deepest block
		for(const ptr<ir::node> input : target->inputs) {
			if(input) {
				const auto it = context.schedule.find(input);

				if(it == context.schedule.end()) {
					// input has no scheduling sus
					continue;
				}

				const ptr<ir::basic_block> basic_block = it->second;

				if(best_depth < basic_block->dominator_depth) {
					best_depth = basic_block->dominator_depth;
					best = basic_block;
				}
			}
		}

		best->items.insert(target);
		context.schedule[target] = best;
	}

	void schedule_late(transformation_context& context, ptr<ir::node> target) {
		constexpr ir::node_id region_id(0, static_cast<u16>(core_node_id::REGION));
		constexpr ir::node_id phi_id(0, static_cast<u16>(core_node_id::PHI));

		// pinned nodes can't be rescheduled 
		if(target->is_pinned()) {
			return;
		}

		// locate the least common ancestor
		ptr<ir::basic_block> lca = nullptr;
		for(ptr<ir::user> user = target->users; user; user = user->next) {
			ptr<ir::node> user_node = user->node;

			auto it = context.schedule.find(user_node);

			if(it == context.schedule.end()) {
				// dead node
				continue;
			}

			ptr<ir::basic_block> use_block = it->second;

			if(user_node->get_id() == phi_id) {
				const ptr<ir::node> use_node = user_node->inputs[0];
				ASSERT(use_node->get_id() == region_id, "unexpected node region PHI input");

				if(user_node->inputs.get_size() != use_node->inputs.get_size() + 1) {
					PANIC("PHI has parent with mismatched predecessors");
				}

				ptrdiff_t j = 1;
				for(; user_node->inputs.get_size(); j++) {
					if(user_node->inputs[j] == target) {
						break;
					}
				}

				it = context.schedule.find(use_node->inputs[j - 1]);

				if(it != context.schedule.end()) {
					use_block = it->second;
				}
			}

			lca = find_lca(lca, use_block);
		}

		if(lca) {
			const auto it = context.schedule.find(target);
			ASSERT(it != context.schedule.end(), "how");
			const ptr<ir::basic_block> old = it->second;

			// replace the old basic block entry
			if(old != lca) {
				it->second = lca;
				old->items.erase(target);
				lca->items.erase(target);
			}
		}
	}
} // namespace baremetal::detail
