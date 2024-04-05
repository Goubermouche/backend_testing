#pragma once
#include "../pass.h"

namespace baremetal {
	class ir_printer_pass : public pass {
	public:
		ir_printer_pass(std::ostream& stream, context& context);

		void apply(module_data& data) override;
	private:
		[[nodiscard]] auto determine_edge_color(ptr<ir::node> node) const -> std::string_view;
		void emit_node_dot(ptr<ir::node> current);
	private:
		std::ostream& m_stream;
		std::unordered_set<ptr<ir::node>> m_visited;

		module_data* m_data;

		ir::node_id m_projection_id;
		ir::node_id m_entry_id;
	};
} // namespace baremetal
