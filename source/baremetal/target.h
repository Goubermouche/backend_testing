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

	class target {
	public:
		target(context& context, const architecture& architecture);
		virtual ~target() = default;

		void initialize_intervals(machine_context& context);
		virtual void select_instructions(machine_context& context) = 0;

		[[nodiscard]] auto get_context() const-> context&;
	private:
		void select_instruction(ptr<ir::node> node) const;
	protected:
		context& m_context;
		architecture m_architecture;

		std::vector<isel_function> m_isel_functions;
	};
} // namespace baremetal
