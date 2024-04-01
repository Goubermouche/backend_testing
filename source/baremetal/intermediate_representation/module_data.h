#pragma once
#include "baremetal/intermediate_representation/node_hierarchy/function.h"

namespace baremetal {
	using namespace utility::types;

	class dialect_base;

	struct module_data : utility::block_allocator {
		module_data();
		~module_data() override;

		auto get_functions() const -> const utility::memory<ptr<ir::function>>&;
		auto get_dialect(u64 id) const -> dialect_base*;
	protected:
		void allocate_function(const ir::function_data_type& data_type);

		utility::memory<ptr<ir::function>> m_functions;
		std::unordered_map<u64, dialect_base*> m_dialect_pointers;
	};
} // namespace baremetal
