#include "x64_target.h"

#include "baremetal/dialects/core_dialect.h"
#include "baremetal/dialects/gpu_dialect.h"

namespace baremetal {
	namespace x64 {
		auto create_architecture() -> architecture {
			architecture arch;

			arch.registers.resize(2);
			arch.registers[GPR].resize(16);
			arch.registers[XMM].resize(16);
		 
			for(u16 i = 0; i < 16; ++i) {
				arch.registers[GPR][i] = { GPR, i };
			}

			for(u16 i = 0; i < 16; ++i) {
				arch.registers[XMM][i] = { XMM, i };
			}

			return arch;
		}

		auto assembler::create_label(ptr<ir::node> node) const -> ptr<instruction> {
			const ptr<instruction> inst = allocate_instruction();

			return inst;
		}

		auto assembler::create_move(ir::data_type dt, reg destination, reg source) const -> ptr<instruction> {
			const ptr<instruction> inst = allocate_instruction();

			return inst;
		}
	} // namespace x64

	x64_target::x64_target(context& context)
	: target(context, x64::create_architecture()), m_context(nullptr) {
		m_isel_functions.resize(context.m_index_map.size());

		m_isel_functions[context.get_dialect_index<core_dialect>()] = { x64::isel_core };
		m_isel_functions[context.get_dialect_index<gpu_dialect>()]  = { x64::isel_gpu  };
	}
	
	void x64_target::select_instructions(machine_context& context) {
		ASSERT(
			context.work_list.get_size() == context.control_flow_graph.get_size(),
			"basic block work list mismatch"
		);

		context.basic_block_order.reserve(context.control_flow_graph.get_size());
		context.values.resize(context.function->get_node_count());

		m_assembler.set_allocator(*context.function);
		m_context = &context;

		define_basic_block_order();
		select_instructions();
	}

	void x64_target::define_basic_block_order() const {
		constexpr ir::node_id exit_id(0, static_cast<u16>(core_node_id::EXIT));
		constexpr ir::node_id phi_id(0, static_cast<u16>(core_node_id::PHI));

		i32 exit_basic_block = -1;

		// define all PHIs early and sort the basic block order
		for(i32 i = 0; i < static_cast<i32>(m_context->control_flow_graph.get_size()); ++i) {
			const ptr<ir::node> basic_block = m_context->work_list[i];
			const ptr<ir::node> exit = m_context->control_flow_graph.at(basic_block).exit;

			for(ptr<ir::user> user = basic_block->users; user; user = user->next) {
				const ptr<ir::node> node = user->node;

				if(
					node->get_id() == phi_id &&
					node->get_data_type().get_id() != static_cast<u8>(ir::data_type_id::MEMORY)
				) {
					m_context->work_list.is_visited(node); // mark the current node as visited
					m_context->values[node->get_global_value_index()] = {
						.use_count = std::numeric_limits<i32>::max()
					};
				}
			}

			if(exit->get_id() == exit_id) {
				exit_basic_block = i;
			}
			else {
				m_context->basic_block_order.push_back(i);
			}
		}

		// the exit block should be the last one we visit
		if(exit_basic_block >= 0) {
			m_context->basic_block_order.push_back(exit_basic_block);
		}
	}

	void x64_target::select_instructions() const {
		for(u64 i = 0; i < m_context->work_list.get_size(); ++i) {
			const ptr<ir::node> basic_block = m_context->work_list[i];

			if(i + 1 < m_context->work_list.get_size()) {
				m_context->fallthrough_label = m_context->basic_block_order[i + 1];
			}
			else {
				m_context->fallthrough_label = std::numeric_limits<i32>::max();
			}

			// create the block label and append it to the instruction list
			const ptr<instruction> label = m_assembler.create_label(basic_block);
			m_context->append_instruction(label);

			// select instructions for the current block
			const ptr<ir::node> block_exit = m_context->control_flow_graph.at(basic_block).exit;
			select_region_instructions(basic_block, block_exit, i);
		}
	}

	void x64_target::greedy_schedule(ptr<ir::basic_block> block, ptr<ir::node> block_exit) const {
		constexpr ir::node_id projection_id(0, static_cast<u16>(core_node_id::PROJECTION));
		constexpr ir::node_id region_id(0, static_cast<u16>(core_node_id::REGION));
		constexpr ir::node_id phi_id(0, static_cast<u16>(core_node_id::PHI));

		const ptr<ir::user> successor = detail::get_next_control_flow_user(block_exit);
		ptr<scheduled_node> top = create_scheduled_node(nullptr, block_exit);

		// phis
		std::vector<scheduled_phi> phis;
		u64 current_phi = 0;

		// leftovers
		const u64 leftover_count = block->items.size();
		u64 leftovers = 0;

		phis.reserve(256);

		if(successor && successor->node->get_id() == region_id) {
			fill_phi_nodes(phis, successor->node, successor->slot);
		}

		m_context->work_list.is_visited(block_exit);

		while(top != nullptr) {
			ptr<ir::node> current = top->node;

			// resolve inputs first
			if(current->get_id() != phi_id && top->index < current->inputs.get_size()) {
				const ptr<ir::node> input = current->inputs[top->index++];

				if(input != nullptr && is_scheduled_in_block(block, input)) {
					top = create_scheduled_node(top, input);
				}

				continue;
			}

			// resolve anti dependencies
			if(top->antis) {
				const ptr<ir::user> next = top->antis->next;
				const ptr<ir::node> anti = top->antis->node;

				if(anti != current && top->antis->slot == 1 && is_scheduled_in_block(block, anti)) {
					top = create_scheduled_node(top, anti);
				}

				top->antis = next;
				continue;
			}

			// resolve phi edges & leftovers when we're at the endpoint
			if(block_exit == current) {
				// skip non-phis
				if(current_phi < phis.size()) {
					const ptr<ir::node> phi = phis[current_phi].phi;
					const ptr<ir::node> val = phis[current_phi].node;

					current_phi++;

					// reserve PHI space
					if(val->get_data_type().get_id() != static_cast<u8>(ir::data_type_id::MEMORY)) {
						phi_value p{
							.phi = phi,
							.node = val,
							.destination = {},
							.source = {}
						};

						m_context->phi_values.push_back(p);
					}

					if(is_scheduled_in_block(block, val)) {
						top = create_scheduled_node(top, val);
					}
					continue;
				}

				// resolve leftover nodes placed here by GCM
				if(leftovers < leftover_count) {
					auto it = block->items.begin();
					std::advance(it, leftovers);

					if(!m_context->work_list.is_visited(it->get())) {
						top = create_scheduled_node(top, it->get());
					}

					leftovers++;
					continue;
				}
			}

			m_context->work_list.push_back(current);
			top = top->parent;

			// push outputs (projections, if they apply)
			if(current->get_data_type().get_id() == static_cast<u8>(ir::data_type_id::TUPLE)) {
				for(ptr<ir::user> user = current->users; user; user = user->next) {
					if(
						user->node->get_id() == projection_id && 
						!m_context->work_list.is_visited(user->node)
					) {
						m_context->work_list.push_back(user->node);
					}
				}
			}
		}
	}

	void x64_target::select_region_instructions(ptr<ir::node> block_entry, ptr<ir::node> block_exit, u64 index) const {
		constexpr ir::node_id projection_id(0, static_cast<u16>(core_node_id::PROJECTION));
		constexpr ir::node_id region_id(0, static_cast<u16>(core_node_id::REGION));
		constexpr ir::node_id entry_id(0, static_cast<u16>(core_node_id::ENTRY));
		constexpr ir::node_id phi_id(0, static_cast<u16>(core_node_id::PHI));

		ASSERT(m_context->work_list.get_size() == m_context->control_flow_graph.get_size(), "basic block work list mismatch");
		const ptr<ir::basic_block> basic_block = m_context->schedule.at(block_entry);
		std::vector<phi_value>& phi_values = m_context->phi_values;

		// topological sort
		greedy_schedule(basic_block, block_exit);

		if(index == 0) {
			for(ptr<ir::user> user = m_context->function->get_entry()->users; user; user = user->next) {
				const ptr<ir::node> node = user->node;

				if(node->get_id() == projection_id && !m_context->work_list.is_visited(node)) {
					// function entry blocks & their projections should be selected as well
					m_context->work_list.push_back(node);
				}
			}
		}

		// define all nodes in this basic block
		for(u64 i = m_context->control_flow_graph.get_size(); i < m_context->work_list.get_size(); ++i) {
			const ptr<ir::node> node = m_context->work_list[i];

			// track non-dead users
			u64 use_count = 0;
			for(ptr<ir::user> user = node->users; user; user = user->next) {
				// user is scheduled == valid user
				if(m_context->schedule.contains(user->node)) {
					use_count++;
				}
			}

			m_context->values[node->get_global_value_index()] = virtual_value{
				.virtual_register = reg{},
				.use_count = static_cast<i32>(use_count),
			};
		}

		// every phi node should view itself as the previous value, not the new one we're producing
		const u64 our_phis = phi_values.size();

		for(u64 i = 0; i < our_phis; ++i) {
			phi_value& value = phi_values[i];
			// value.destination = get_virtual_register(value.phi);
		}

		if(block_entry->get_id() == region_id) {
			for(ptr<ir::user> user = block_entry->users; user; user = user->next) {
				if(
					user->node->get_id() == phi_id &&
					user->node->get_data_type().get_id() != static_cast<u8>(ir::data_type_id::MEMORY)
				) {
					const ptr<virtual_value> value = &m_context->values[user->node->get_global_value_index()];

					// copy the PHI node into a temporary
					phi_value phi = {
						.phi = user->node,
						// .destination = get_virtual_register(user->node)
					};

					phi_values.push_back(phi);

					const ir::data_type dt = phi.phi->get_data_type();
					const reg temp = /*allocate_virtual_register(nullptr, dt);*/ {};
					m_context->append_instruction(m_assembler.create_move(dt, temp, phi.destination));

					// assign temporary as the PHI until the end of the BB
					value->virtual_register = temp;
				}
			}
		}

		if(index == 0) {
			// select instructions for the entry node
			select_instruction(m_context->function->get_entry(), reg{});
		}

		// walk all nodes
		const ptr<instruction> current = m_context->current_instruction;
		ptr<instruction> last = nullptr;

		for(u64 i = m_context->work_list.get_size(); i-- > m_context->control_flow_graph.get_size();) {
			const ptr<ir::node> node = m_context->work_list[i];

			if(node->get_id() == entry_id) {
				continue;
			}

			// const ptr<virtual_value> value = m_context->get_virtual_value(node);
			// 
			// if(node != block_exit && !value->virtual_register.is_valid() && node->should_rematerialize()) {
			// 	continue;
			// }
			// 
			// // attach to dummy list
			// instruction dummy;
			// dummy.next = nullptr;
			// m_context->current_instruction = &dummy;
			// 
			// if(node->dt.kind == type_kind::TUPLE || node->dt.kind == type_kind::CONTROL || node->dt.kind == type_kind::MEMORY) {
			// 	select_instruction(node, value->virtual_register);
			// }
			// else if(value->use_count > 0 || value->virtual_register.is_valid()) {
			// 	if(!value->virtual_register.is_valid()) {
			// 		value->virtual_register = allocate_virtual_register(node, node->dt);
			// 	}
			// 
			// 	select_instruction(node, value->virtual_register);
			// }
			// else {
			// 	// dead
			// }
			// 
			// handle<instruction> sequence_start = dummy.next;
			// const handle<instruction> sequence_end = m_context->current_instruction;
			// ASSERT(sequence_end->next == nullptr, "invalid instruction detected");
			// 
			// if(sequence_start != nullptr) {
			// 	if(last == nullptr) {
			// 		last = sequence_end;
			// 		current->next = dummy.next;
			// 	}
			// 	else {
			// 		const handle<instruction> old_next = current->next;
			// 		current->next = sequence_start;
			// 		sequence_end->next = old_next;
			// 	}
			// }
		}



		// reset
		phi_values.clear();
		m_context->work_list.set_size(m_context->control_flow_graph.get_size());
	}

	auto x64_target::create_scheduled_node(ptr<scheduled_node> parent, ptr<ir::node> node) const -> ptr<scheduled_node> {
		constexpr ir::node_id projection_id(0, static_cast<u16>(core_node_id::PROJECTION));
		constexpr ir::node_id phi_id(0, static_cast<u16>(core_node_id::PHI));

		const auto scheduled = m_context->function->emplace<scheduled_node>();

		scheduled->parent = parent;
		scheduled->node = node;
		scheduled->index = 0;

		if(
			node->is_memory_out_operator() && 
			node->get_id() != phi_id && 
			node->get_id() != projection_id
		) {
			scheduled->antis = node->inputs[1]->users;
		}

		return scheduled;
	}

	void x64_target::fill_phi_nodes(std::vector<scheduled_phi>& phi_nodes, ptr<ir::node> successor, i32 phi_index) {
		constexpr ir::node_id phi_id(0, static_cast<u16>(core_node_id::PHI));

		for(ptr<ir::user> user = successor->users; user; user = user->next) {
			if(user->node->get_id() != phi_id) {
				continue;
			}

			phi_nodes.push_back({
				.phi = user->node,
				.node = user->node->inputs[1 + phi_index]
			});
		}
	}

	auto x64_target::is_scheduled_in_block(ptr<ir::basic_block> block, ptr<ir::node> node) const -> bool {
		const auto it = m_context->schedule.find(node);
		return it != m_context->schedule.end() && it->second == block && !m_context->work_list.is_visited(node);
	}
} // namespace baremetal

