#include "ir_printer_pass.h"

#include "baremetal/dialects/core_dialect.h"

namespace baremetal {
	ir_printer_pass::ir_printer_pass(std::ostream& stream, context& context) : m_stream(stream), m_data(nullptr) {
		const u8 core_index = context.get_dialect_index<core_dialect>();

		m_projection_id = ir::node_id(core_index, static_cast<u16>(core_node_id::PROJECTION));
		m_entry_id = ir::node_id(core_index, static_cast<u16>(core_node_id::ENTRY));
	}

	void ir_printer_pass::apply(module_data& data) {
		m_data = &data;

		m_stream << "digraph {\n";
		m_stream << "  node [style=filled, fillcolor=white]\n";

		// emit dot code for all functions
		for(u64 i = 0; i < data.get_functions().get_size(); ++i) {
			const ptr<ir::function> current = data.get_functions()[i];
			m_stream << std::format("  subgraph cluster_func{}{{\n", i + 1);
			m_stream << std::format("    label=\"Function {}\"\n", i + 1);

			// begin from the exit nodes
			for(const ptr<ir::node> exit : current->terminators) {
				emit_node_dot(exit);
			}

			m_stream << "  }\n";
			m_visited.clear();
		}

		m_stream << "}\n";
	}

	auto ir_printer_pass::determine_edge_color(ptr<ir::node> node) const -> std::string_view {
		switch(node->get_data_type().get_id()) {
			case static_cast<u8>(ir::data_type_id::CONTROL):      return "red";
			case static_cast<u8>(ir::data_type_id::MEMORY):       return "blue";
			case static_cast<u8>(ir::data_type_id::CONTINUATION): return "purple";
			default: {
				return m_data->get_dialect(node->get_id().get_dialect_id())->get_color(node);
			}
		}
	}

	void ir_printer_pass::emit_node_dot(ptr<ir::node> current) {
		constexpr std::string_view projection_edge_fmt = "    n{}:p{} -> n{}:i{} [color={}, edgetooltip=\"PROJECTION n{} -> p{} -> n{}\"];\n";
		constexpr std::string_view edge_fmt = "    n{} -> n{}:i{} [color={}, edgetooltip=\"n{} -> n{}\"];\n";

		constexpr std::string_view label_begin_fmt = "    n{} [ordering=in, shape=record, tooltip=\"n{}\", label=";
		constexpr std::string_view entry_label_fmt = "\"{{{}|{{<p0> ctrl|<p1> mem|<p2> cont}}}}\"]\n";

		// skip visited nodes
		if(m_visited.contains(current)) {
			return;
		}

		m_visited.insert(current);

		// handle projections
		if(current->get_id() == m_projection_id) {
			return;
		}

		// print the current node's label
		m_stream << std::format(label_begin_fmt, current, current->get_global_value_index());

		if(current->get_id() == m_entry_id) {
			// entry nodes have a preset formatter
			m_stream << std::format(entry_label_fmt, m_data->get_dialect(current->get_id().get_dialect_id())->get_label(current));
		}
		else if(current->inputs.is_empty()) {
			// nodes without inputs don't need input sections
			m_stream << std::format("\"{}\"]\n", m_data->get_dialect(current->get_id().get_dialect_id())->get_label(current));
		}
		else {
			// print the input section
			m_stream << "\"{{";

			for(u8 j = 0; j < current->inputs.get_size(); ++j) {
				m_stream << std::format("<i{}> {}", j, j);

				if(j + 1 < current->inputs.get_size()) {
					m_stream << "|";
				}
			}

			m_stream << std::format("}}|{}}}\"]\n", m_data->get_dialect(current->get_id().get_dialect_id())->get_label(current));
		}

		// print edges to the input nodes
		u64 current_gvn = current->get_global_value_index();

		for(u8 i = 0; i < current->inputs.get_size(); ++i) {
			if(const ptr<ir::node> input = current->inputs[i]) {
				std::string_view edge_color = determine_edge_color(input);

				if(input->get_id() != m_projection_id) {
					u64 input_gvn = input->get_global_value_index();

					m_stream << std::format(edge_fmt, input, current, i, edge_color, input_gvn, current_gvn);
				}
				else {
					u64 input_gvn = input->inputs[0]->get_global_value_index();
					u64 projection_gvn = input->get_global_value_index();

					m_stream << std::format(projection_edge_fmt, input->inputs[0], i, current, i, edge_color, input_gvn, projection_gvn, current_gvn);
				}

				// emit dot code for the input nodes
				emit_node_dot(input);
			}
		}
	}
} // namespace baremetal
