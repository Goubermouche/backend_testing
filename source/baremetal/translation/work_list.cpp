#include "work_list.h"

#include "baremetal/dialects/core_dialect.h"

namespace baremetal {
	void work_list::push_all(ptr<ir::function> function) {
		memory stack;

		// collect all nodes, starting from the terminators
		for(const ptr<ir::node> exit : function->m_terminators) {
			if(is_visited(exit)) {
				// skip visited nodes
				continue;
			}

			stack.push_back(exit);

			while(!stack.is_empty()) {
				ptr<ir::node> current = stack.pop_back();

				// place self first
				push_back(current);

				for(const ptr<ir::node> input : current->inputs) {
					if(input && !is_visited(input)) {
						stack.push_back(input);
					}
				}
			}
		}
	}

	void work_list::compute_dominators(control_flow_graph& cfg) {
		const ptr entry = &cfg.at(m_data[0]);
		bool changed = true;

		// the entry node dominates itself
		entry->dominator = entry;

		while(changed) {
			changed = false;

			// ignore the entry node
			for(u64 i = 1; i < cfg.get_size(); ++i) {
				ptr<ir::node> new_immediate_dominator = nullptr;
				const ptr<ir::node> block = m_data[i];

				// pick first "processed" predecessor
				const u8 predecessor_count = block->inputs.get_size();
				u8 j = 0;

				for(; j < predecessor_count; j++) {
					const ptr<ir::node> predecessor = detail::get_predecessor(block, j);

					if(cfg.get_immediate_dominator(predecessor) != nullptr) {
						new_immediate_dominator = predecessor;
						break;
					}
				}

				// for all other predecessors, p, of b
				for(; j < predecessor_count; j++) {
					const ptr<ir::node> predecessor = detail::get_predecessor(block, j);

					// if the predecessor has a dominator
					if(cfg.get_immediate_dominator(predecessor)) {
						u64 a = cfg.get_traversal_index(predecessor);

						// if the traversal index is valid
						if(a != std::numeric_limits<u64>::max()) {
							const u64 b = cfg.get_traversal_index(new_immediate_dominator);

							// while (finger1 < finger2)
							//   finger1 = dominators[finger1]
							while(a > b) {
								const ptr<ir::node> dominator = cfg.get_immediate_dominator(m_data[a]);
								a = dominator ? cfg.get_traversal_index(dominator) : 0;
							}

							// while (finger2 < finger1)
							//   finger2 = dominators[finger2]
							while(b > a) {
								const ptr<ir::node> dominator = cfg.get_immediate_dominator(m_data[b]);
								a = dominator ? cfg.get_traversal_index(dominator) : 0;
							}
						}

						new_immediate_dominator = m_data[a];
					}
				}

				ASSERT(new_immediate_dominator != nullptr, "invalid dominator");
				const auto block_basic_block = &cfg.at(block);

				// we've calculated a dominator for the current basic block
				if(
					block_basic_block->dominator == nullptr ||
					block_basic_block->dominator->entry != new_immediate_dominator
					) {
					block_basic_block->dominator = &cfg.at(new_immediate_dominator);
					changed = true;
				}
			}
		}

		// recalculate the dominator depth for all basic blocks
		for(u64 i = cfg.get_size(); i-- > 1;) {
			cfg.resolve_dominator_depth(m_data[i]);
		}
	}

	auto work_list::is_visited(ptr<ir::node> node) -> bool {
		return !visited.insert(node).second;
	}
} // namespace baremetal
