#pragma once
#include "baremetal/translation/translation_engine.h"
#include "baremetal/translation/reg.h"
#include "baremetal/context.h"

namespace baremetal {
	inline void default_isel(ptr<ir::node>) {
		utility::console::out("unknown dialect\n");
	}

	struct isel_function {
		using func = std::function<void(ptr<ir::node>)>;

		func function = default_isel;
	};

	struct architecture {
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
		[[nodiscard]] auto allocate_instruction() const -> ptr<instruction> {
			const auto inst = m_allocator->emplace<instruction>();
			return inst;
		}
	protected:
		utility::block_allocator* m_allocator;
	};

	class target {
	public:
		target(context& context, const architecture& architecture);
		virtual ~target() = default;

		void initialize_intervals(machine_context& context);
		virtual void select_instructions(machine_context& context) = 0;

		[[nodiscard]] auto get_context() const-> context&;
	protected:
		void select_instruction(ptr<ir::node> node, reg reg) const;
	protected:
		context& m_compilation_context;
		architecture m_architecture;

		std::vector<isel_function> m_isel_functions;
	};
} // namespace baremetal
