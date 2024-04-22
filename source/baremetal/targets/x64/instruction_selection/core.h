#pragma once
#include "baremetal/dialects/core_dialect.h"
#include "baremetal/targets/x64/architecture.h"

namespace baremetal {
	class x64_target;

	namespace x64 {
		void select_instruction_core(ptr<target> target, ptr<ir::node> node, reg destination);

		void select_instruction_core_entry(ptr<x64_target> target, ptr<ir::node> node);
		void select_instruction_core_exit(ptr<x64_target> target, ptr<ir::node> node);
		void select_instruction_core_projection(ptr<x64_target> target, ptr<ir::node> node, reg destination);
		void select_instruction_core_memory_block(ptr<x64_target> target, ptr<ir::node> node, reg destination);
		void select_instruction_core_store(ptr<x64_target> target, ptr<ir::node> node, reg destination);
		void select_instruction_core_load(ptr<x64_target> target, ptr<ir::node> node, reg destination);
		void select_instruction_core_integer(ptr<x64_target> target, ptr<ir::node> node, reg destination);

		auto select_instruction_core_address(ptr<x64_target> target, ptr<ir::node> node, reg destination, instruction_id id, reg source = {}) -> ptr<instruction>;
	} // namespace x64
} // namespace baremetal
