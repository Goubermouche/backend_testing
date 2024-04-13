#include "translation_engine.h"

#include "baremetal/translation/mandatory_passes.h"
#include "baremetal/target.h"

namespace baremetal {
	// void translation_engine::translate(core::target& target) {
	// 	for(const managed_ptr<pass> pass : m_passes) {
	// 		pass->apply(target);
	//   }
	// 
	// 	core::codegen_context context {
	// 		.function = target.get_current_function()
	// 	};
	// 
	// 	generate_user_lists(context);
	// 	schedule_ir_graph(context);
	// 
	// 	target.select_instructions(context);
	// 
	// 	// utility::binary_buffer stream;
	// 	// 
	// 	// stream.push_back("test");
	// 	// stream.push_back(156);
	// 	// 
	// 	// return stream;
	// }

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
