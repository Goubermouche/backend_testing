#pragma once
#include "baremetal/intermediate_representation/module_data.h"
#include "baremetal/dialects/core_dialect.h"

namespace baremetal {
	inline void default_isel(ptr<ir::node>) {
		std::cout << "unknown dialect\n";
	}

	struct isel_function {
		using func = std::function<void(ptr<ir::node>)>;

		func function = default_isel;
	};

	class target {
	public:
		target(context& context);

		virtual void select_instructions(const module_data& data) = 0;

		[[nodiscard]] auto get_context() const-> context&;
	protected:
		std::vector<isel_function> m_isel_functions;
		context& m_context;
	};
} // namespace baremetal
