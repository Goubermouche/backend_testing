#include <baremetal/intermediate_representation/module.h>
#include <baremetal/targets/x64_target.h>
#include <baremetal/translation_engine.h>
#include <baremetal/passes/ir_printer_pass.h>

#include <baremetal/dialects/core_dialect.h>
#include <baremetal/dialects/gpu_dialect.h>

using namespace utility::types;

// TODO: flags
// TODO: block documentation
// register stuff can be generic?
// timings
// pointers instead of unique ids?
// modification of already existing targets for certain dialects 

auto main() -> int {
	baremetal::module<baremetal::core_dialect, baremetal::gpu_dialect> module;

	module.create_function({});

	{
		using namespace baremetal;

		const ptr<ir::node> local = module.create_local(4, 8);
		const ptr<ir::node> imm = module.create_signed_integer(123, 32);
		const ptr<ir::node> test = module.create_gpu_test(imm);

		module.create_store(local, test, 8);
		module.create_ret({ module.create_load(local, core_dialect::I32_TYPE, 8) });
	}

	baremetal::translation_engine engine;
	baremetal::x64_target target;

	std::ofstream ir_graph("./graph.dot");

	engine.add_pass(managed_ptr<baremetal::ir_printer_pass>(module, ir_graph));
	engine.translate(target, module);

	ir_graph.close();

	return 0;
}
