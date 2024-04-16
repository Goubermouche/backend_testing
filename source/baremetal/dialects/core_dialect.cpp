#include "core_dialect.h"

namespace baremetal {
	auto core_dialect::get_label(ptr<ir::node> node) const -> std::string_view {
		switch(node->get_id().get_node_id()) {
			case static_cast<u16>(core_node_id::ENTRY):        return "entry";
			case static_cast<u16>(core_node_id::EXIT):         return "exit";
			case static_cast<u16>(core_node_id::REGION):       return "region";

			case static_cast<u16>(core_node_id::PROJECTION):   return "projection";
			case static_cast<u16>(core_node_id::PHI):          return "phi";

			case static_cast<u16>(core_node_id::MEMORY_BLOCK): return "memory block";
			case static_cast<u16>(core_node_id::STORE):        return "store";
			case static_cast<u16>(core_node_id::LOAD):         return "load";
			case static_cast<u16>(core_node_id::INTEGER_IMM):  return "integer";
			default: return "UNKNOWN";
		}
	}

	void core_dialect::create_function(const ir::function_data_type& data_type) {
		allocate_function(data_type);

		const auto& parameters = data_type.parameter_types;
		const	ptr<ir::function> function = m_functions.last();

		function->set_entry(allocate_node<memory_region>(static_cast<u16>(core_node_id::ENTRY), 0, ir::TUPLE_TYPE));
		function->get_entry()->flags |= ir::IS_PINNED;

		// fill in entry projections
		function->set_active_control(create_projection(ir::CONTROL_TYPE, function->get_entry(), 0));
		function->parameters = utility::memory_view<ptr<ir::node>>(*this, 3 + parameters.get_size());

		function->parameters[0] = function->get_active_control();
		function->parameters[1] = create_projection(ir::MEMORY_TYPE, function->get_entry(), 1);
		function->parameters[2] = create_projection(ir::CONTINUATION_TYPE, function->get_entry(), 2);

		for(u8 i = 0; i < parameters.get_size(); ++i) {
			function->parameters[3 + i] = create_projection(parameters[i], function->get_entry(), 3 + i);
		}

		// initialize memory io
		memory_region& region = function->get_entry()->get_data<memory_region>();
		region.output_memory = function->parameters[1];
		region.input_memory = function->parameters[1];
	}

	auto core_dialect::create_local(u8 alignment, u8 size) -> ptr<ir::node> {
		const auto memory_node = allocate_node<memory_block>(static_cast<u16>(core_node_id::MEMORY_BLOCK), 1, ir::data_type{});

		memory_node->inputs[0] = m_functions.last()->get_entry();
		// memory_node->flags |= node_flags::SHOULD_REMATERIALIZE;

		memory_block& memory = memory_node->get_data<memory_block>();
		memory.alignment = alignment;
		memory.size = size;

		return memory_node;
	}

	auto core_dialect::create_signed_integer(i64 value, u8 bit_width) -> ptr<ir::node> {
		const auto immediate_node = allocate_node<integer_immediate>(static_cast<u16>(core_node_id::INTEGER_IMM), 1, ir::data_type(100, bit_width));
		immediate_node->flags |= ir::node_flags::SHOULD_REMATERIALIZE;

		integer_immediate& immediate = immediate_node->get_data<integer_immediate>();
		immediate.value = value;

		return immediate_node;
	}

	void core_dialect::create_store(ptr<ir::node> destination, ptr<ir::node> value, u8 alignment) {
		const auto store_node = allocate_node<projection>(static_cast<u16>(core_node_id::STORE), 4, ir::MEMORY_TYPE);

		store_node->inputs[0] = m_functions.last()->get_active_control();
		store_node->inputs[1] = append_memory(store_node);
		store_node->inputs[2] = destination;
		store_node->inputs[3] = value;

		store_node->get_data<memory_access>().alignment = alignment;
	}

	auto core_dialect::create_load(ptr<ir::node> source, ir::data_type type, u8 alignment) -> ptr<ir::node> {
		const auto load_node = allocate_node<memory_access>(static_cast<u16>(core_node_id::LOAD), 3, type);

		load_node->inputs[0] = m_functions.last()->get_active_control();
		load_node->inputs[1] = get_parent_region(m_functions.last()->get_active_control())->get_data<memory_region>().output_memory;
		load_node->inputs[2] = source;

		load_node->get_data<memory_access>().alignment = alignment;
		return load_node;
	}

	void core_dialect::create_ret(const utility::memory<ptr<ir::node>, u8>& return_values) {
		const ptr<ir::function> function = m_functions.last();

		ASSERT(function->get_exit() == nullptr, "not implemented");

		const ptr<ir::node> memory_state = get_parent_region(function->get_active_control())->get_data<memory_region>().output_memory;
		const ptr<ir::node> region = allocate_node<memory_region>(static_cast<u16>(core_node_id::REGION), 0, ir::CONTROL_TYPE);
		region->flags |= ir::IS_PINNED;

		const ptr<ir::node> memory_phi = allocate_node(static_cast<u16>(core_node_id::PHI), 2, ir::MEMORY_TYPE);
		memory_phi->inputs[0] = region;
		memory_phi->inputs[1] = memory_state;
		memory_phi->flags |= ir::IS_PINNED;

		function->set_exit(allocate_node(static_cast<u16>(core_node_id::EXIT), 3 + return_values.get_size(), ir::CONTROL_TYPE));
		function->get_exit()->inputs[0] = region;
		function->get_exit()->inputs[1] = memory_phi;
		function->get_exit()->inputs[2] = function->parameters[2];
		function->get_exit()->flags |= ir::IS_PINNED;
		function->get_exit()->flags |= ir::IS_CONTROL_FLOW_TERMINATOR;
		function->get_exit()->flags |= ir::IS_CONTROL_FLOW_ENDPOINT;

		// return values
		for(u8 i = 0; i < return_values.get_size(); ++i) {
			const ptr<ir::node> phi_node = allocate_node(static_cast<u16>(core_node_id::PHI), 2, return_values[i]->get_data_type());
			phi_node->inputs[0] = region;
			phi_node->inputs[1] = return_values[i];
			phi_node->flags |= ir::IS_PINNED;

			function->get_exit()->inputs[3 + i] = phi_node;
		}

		memory_region& memory = region->get_data<memory_region>();
		memory.output_memory = memory_phi;
		memory.input_memory = memory_phi;

		function->terminators.push_back(function->get_exit());

		const ptr<ir::node> control = function->get_exit()->inputs[0];
		append_input(control, function->get_active_control());
		function->set_active_control(nullptr);
	}

	auto core_dialect::create_projection(ir::data_type type, ptr<ir::node> source, u8 index) -> ptr<ir::node> {
		ASSERT(source->get_data_type().get_id() == static_cast<u8>(ir::data_type_id::TUPLE), "invalid type id, expected a tuple");
		const ptr<ir::node> projection_node = allocate_node<projection>(static_cast<u16>(core_node_id::PROJECTION), 1, type);

		projection_node->get_data<projection>().index = index;
		projection_node->inputs[0] = source;
		projection_node->flags |= ir::IS_PINNED;

		return projection_node;
	}

	auto core_dialect::append_memory(ptr<ir::node> new_memory) const -> ptr<ir::node> {
		const auto basic_block = get_parent_region(m_functions.last()->get_active_control());
		memory_region& region = basic_block->get_data<memory_region>();

		const ptr<ir::node> old_memory = region.output_memory;
		region.output_memory = new_memory;
		return old_memory;
	}

	void core_dialect::append_input(ptr<ir::node> target, ptr<ir::node> input) {
		// detach the old predecessor memory, make a bigger one
		const u8 old_count = target->inputs.get_size();
		utility::memory_view<ptr<ir::node>, u8> new_inputs(*m_functions.last(), old_count + 1);
		utility::copy(new_inputs, target->inputs);

		new_inputs[old_count] = input;
		target->inputs = new_inputs;
	}

	auto core_dialect::get_parent_region(ptr<ir::node> node) -> ptr<ir::node> {
		constexpr ir::node_id region_id(0, static_cast<u16>(core_node_id::REGION));
		constexpr ir::node_id entry_id(0, static_cast<u16>(core_node_id::ENTRY));

		while(node->get_id() != region_id && node->get_id() != entry_id) {
			node = node->inputs[0];
		}

		return node;
	}
} // namespace baremetal
