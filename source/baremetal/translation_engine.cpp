#include "translation_engine.h"

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

		target.select_instructions(module);
	}
} // namespace baremetal
