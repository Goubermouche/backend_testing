#include "target.h"

namespace baremetal {
	auto architecture::get_address_slot(u8 index, ptr<ir::node> node) -> i32 {
		return address_spaces[index].get_slot(node);
	}

	target::target(context& context, const baremetal::architecture& architecture)
	: architecture(architecture), m_compilation_context(context){}

	void target::initialize_intervals(machine_context& context) const {
		u64 register_count = 0;

		for(const auto& registers : architecture.registers) {
			register_count += registers.size();
		}

		context.intervals.reserve(register_count);

		for(u64 i = 0; i < architecture.registers.size(); ++i) {
			const auto& registers = architecture.registers[i];

			for(u64 j = 0; j < registers.size(); ++j) {
				context.intervals.emplace_back(std::vector{ utility::range<i32>::max() }, registers[j]);
			}
		}
	}

	void target::select_instruction(ptr<ir::node> node, reg reg) {
		m_isel_functions[node->get_id().get_dialect_id()].function(this, node, reg);
	}

	assembler::assembler() : m_allocator(nullptr) {}

	void assembler::set_allocator(utility::block_allocator& allocator) {
		m_allocator = &allocator;
	}
} // namespace baremetal
