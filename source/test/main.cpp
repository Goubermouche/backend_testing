#include <baremetal/intermediate_representation/module.h>
#include <baremetal/translation/translation_engine.h>
#include <baremetal/passes/ir_printer_pass.h>
#include <baremetal/targets/x64/x64_target.h>

#include <baremetal/dialects/gpu_dialect.h>

using namespace utility::types;

// TODO: block documentation
// TODO: 
// register stuff can be generic?
// timings
// block printer
// dumb ir printer
// ir printer
// block printer

auto main() -> int {
	baremetal::context context;
	baremetal::module<baremetal::gpu_dialect> module(context);

	{
		using namespace baremetal;

		module.create_function({});

		const ptr<ir::node> local = module.create_local(8, 8);
		const ptr<ir::node> imm = module.create_signed_integer(std::numeric_limits<i64>::max(), 64);
		// const ptr<ir::node> test = module.create_gpu_test(imm);

		module.create_store(local, imm, 8);
		module.create_ret({ module.create_load(local, ir::I64_TYPE, 8) });
	}

	baremetal::translation_engine engine;
	baremetal::x64_target target(context);

	std::ofstream ir_graph("./graph.dot");

	engine.add_pass(managed_ptr<baremetal::ir_printer_pass>(module, ir_graph, context));
	engine.translate(target, module);

	ir_graph.close();

	return 0;
}
