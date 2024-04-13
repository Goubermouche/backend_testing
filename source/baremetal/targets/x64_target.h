#pragma once
#include "baremetal/target.h"

namespace baremetal {
	namespace x64 {
		enum register_class : u8 {
			GPR = 0,
			XMM
		};

		enum gpr_register : u16 {
			RAX = 0,
			RCX,
			RDX,
			RBX,
			RSI,
			RDI,
			RSP,
			RBP,
			R8,
			R9,
			R10,
			R11,
			R12,
			R13,
			R14,
			R15
		};

		enum xmm_register : u16 {
			XMM0 = 0,
			XMM1,
			XMM2,
			XMM3,
			XMM4,
			XMM5,
			XMM6,
			XMM7,
			XMM8,
			XMM9,
			XMM10,
			XMM11,
			XMM12,
			XMM13,
			XMM14,
			XMM15,
		};

		inline void isel_core(ptr<ir::node>) {
			std::cout << "core\n";
		}

		inline void isel_gpu(ptr<ir::node>) {
			std::cout << "gpu\n";
		}

		auto create_architecture() -> architecture;
	} // namespace x64

	class x64_target : public target {
	public:
		x64_target(context& context);

		void select_instructions(machine_context& context) override;
	};
} // namespace baremetal
