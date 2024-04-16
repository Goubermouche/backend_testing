#pragma once
#include "baremetal/intermediate_representation/module_data.h"

#include <utility/typeid.h>

namespace baremetal {
	class dialect_base;

	template<typename type>
	concept derived_from_dialect_base = std::is_base_of_v<dialect_base, type>;

	template<derived_from_dialect_base... dialects>
	class module;

	class dialect_base : virtual public module_data {
	public:
		[[nodiscard]] virtual auto get_label(ptr<ir::node> node) const -> std::string_view;
	protected:
		u8 m_index = 0; // unique index within the module::dialects array

		template<derived_from_dialect_base... dialects>
		friend class module;
	};

	class dialect : public dialect_base {
	public:
		dialect() = default;
	protected:
		template<typename node_data = std::monostate>
		[[nodiscard]] auto allocate_node(u16 node_id, u8 input_count, ir::data_type dt) -> ptr<ir::node> {
			return m_functions.last()->allocate_node<node_data>(
				{ m_index, node_id }, input_count, dt
			);
		}
	};
} // namespace baremetal
