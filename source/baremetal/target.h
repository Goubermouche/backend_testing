#pragma once
#include "baremetal/translation/translation_engine.h"
#include "baremetal/translation/address_space.h"
#include "baremetal/translation/reg.h"
#include "baremetal/context.h"

namespace baremetal {
	inline void default_isel(ptr<target>, ptr<ir::node>, reg) {
		utility::console::out("unknown dialect\n");
	}

	struct isel_function {
		using func = std::function<void(ptr<target>, ptr<ir::node>, reg)>;

		func function = default_isel;
	};

	struct architecture {
		auto get_address_slot(u8 index, ptr<ir::node> node) -> i32;

		std::vector<address_space> address_spaces;

		// list of all available register for a given arch, ie.:
		// GPR { RAX, RCX, RDX, RBX, RSI, ... }
		// XMM { XMM0, XMM1, XMM2, XMM3, XMM4, ... }
		std::vector<std::vector<reg>> registers;
	};

	class assembler {
	public:
		assembler();

		void set_allocator(utility::block_allocator& allocator);
	protected:
		[[nodiscard]] auto allocate_instruction(u8 out, u8 in, u8 temp) const -> ptr<instruction> {
			const auto inst = m_allocator->emplace<instruction>(out, in, temp);
			inst->m_operands = utility::memory_view<reg, u8>(*m_allocator, inst->get_operand_count());

			return inst;
		}
	protected:
		utility::block_allocator* m_allocator;
	};

	class target {
	public:
		target(context& context, const architecture& architecture);
		virtual ~target() = default;

		void initialize_intervals(machine_context& context) const;
		virtual void select_instructions(machine_context& context) = 0;
	protected:
		void select_instruction(ptr<ir::node> node, reg reg);
	public:
		architecture architecture;
	protected:
		context& m_compilation_context;
		std::vector<isel_function> m_isel_functions;
	};
} // namespace baremetal
