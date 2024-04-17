#pragma once
#include "baremetal/intermediate_representation/node_hierarchy/function.h"

namespace baremetal {
	using namespace utility::types;

	class dialect_base;

	struct module_data : utility::block_allocator {
		module_data();
		~module_data() override;

		[[nodiscard]] auto get_functions() const -> const utility::memory<ptr<ir::function>>&;
		[[nodiscard]] auto get_dialect(u64 index) const -> ptr<dialect_base>;
	protected: 
		void allocate_function(const ir::function_data_type& data_type);
	protected:
		utility::memory<ptr<ir::function>> m_functions;
		utility::memory<ptr<dialect_base>, u8> m_dialects;
	};
} // namespace baremetal
