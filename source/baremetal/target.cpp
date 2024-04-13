#include "target.h"

namespace baremetal {
	target::target(context& context, const architecture& architecture)
	: m_context(context), m_architecture(architecture){}

	void target::initialize_intervals(machine_context& context) {
		u64 register_count = 0;

		for(const auto& registers : m_architecture.registers) {
			register_count += registers.size();
		}

		context.intervals.reserve(register_count);

		for(u64 i = 0; i < m_architecture.registers.size(); ++i) {
			const auto& registers = m_architecture.registers[i];

			for(u64 j = 0; j < registers.size(); ++j) {
				context.intervals.emplace_back(std::vector{ utility::range<i32>::max() }, registers[j]);
			}
		}
	}

	auto target::get_context() const -> context& {
		return m_context;
	}

	void target::select_instruction(ptr<ir::node> node) const {
		m_isel_functions[node->get_id().get_dialect_id()].function(node);
	}
} // namespace baremetal
