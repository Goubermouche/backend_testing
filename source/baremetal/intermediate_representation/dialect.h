#pragma once
#include "baremetal/intermediate_representation/module_data.h"
#include <utility/typeid.h>

namespace baremetal {


	class dialect_base;

	template<typename type>
	concept derived_from_dialect_base = std::is_base_of_v<dialect_base, type>;

	class dialect_base : virtual public module_data {
	public:
		virtual auto get_label(u16 id) const->std::string_view;
		virtual auto get_color(u16 id) const->std::string_view;
		virtual ~dialect_base() = default;
	};

	template<typename dialect_type>
	class dialect : public dialect_base{
	public:
		dialect() = default;
	protected:
		template<typename node_data = std::monostate>
		auto allocate_node(u16 nodeID, u8 input_count, ir::data_type dt) -> ptr<ir::node> {
			std::cout << utility::get_type_id<dialect_type>() << '\n';

			return m_functions.last()->allocate_node<node_data>({ utility::get_type_id<dialect_type>(), nodeID }, input_count, dt);
		}
	};
} // namespace baremetal
