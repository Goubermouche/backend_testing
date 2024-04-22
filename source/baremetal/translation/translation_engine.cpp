#include "translation_engine.h"

#include "baremetal/translation/mandatory_passes.h"
#include "baremetal/target.h"

namespace baremetal {
	void machine_context::append_instruction(ptr<instruction> instruction) {
		if(first_instruction == nullptr) {
			first_instruction = instruction;
			current_instruction = instruction;
		}
		else {
			current_instruction->next = instruction;
			current_instruction = instruction;
		}
	}

	auto machine_context::get_virtual_value(ptr<ir::node> node) -> ptr<virtual_value> {
		if(work_list.visited.contains(node)) {
			return &values[node->get_global_value_index()];
		}

		return nullptr;
	}

	void machine_context::use_node(ptr<ir::node> node) {
		if(const ptr<virtual_value> value = get_virtual_value(node)) {
			value->use_count--;
		}
	}

	void translation_engine::add_pass(managed_ptr<pass> pass) {
		m_passes.push_back(pass);
	}

	void translation_engine::translate(target& target, module_data& module) {
		for(const managed_ptr<pass> pass : m_passes) {
			pass->apply(module);
		}

		for(const ptr<ir::function> function : module.get_functions()) {
			transformation_context transformation{
				.function = function
			};

			// apply mandatory transformation passes
			detail::generate_use_lists(transformation);
			detail::schedule_function(transformation);

			machine_context machine {
				.function = function,
				.schedule = std::move(transformation.schedule),
				.control_flow_graph = std::move(transformation.control_flow_graph),
				.work_list = std::move(transformation.work_list)
			};

			target.initialize_intervals(machine);
			target.select_instructions(machine);



	//  detail::calculate_live_ranges
	//  detail::calculate_intervals
		}

		// target.select_instructions(module);
	}
} // namespace baremetal
