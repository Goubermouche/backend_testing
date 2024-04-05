#pragma once
#include "baremetal/intermediate_representation/dialect.h"
#include "baremetal/context.h"

namespace baremetal {
	struct memory_block {
		u8 alignment;     // bytes
		u8 size;          // bytes
	};

	struct memory_access {
		u8 alignment;
	};

	struct memory_region {
		ptr<ir::node> input_memory;
		ptr<ir::node> output_memory;
	};

	struct projection {
		u8 index;
	};

	enum class core_node : u16 {
		ENTRY,        // function entry node
		EXIT,         // function exit node
		REGION,

		PROJECTION,   // projection for a value
		PHI,          // conditionally defined value

		MEMORY_BLOCK, // generic memory block allocated in the specified address space 
		STORE,        // generic store operation
		LOAD,         // generic load operation

		INTEGER_IMM
	};

	struct integer_immediate {
		u64 value;
	};

	class core_dialect : public dialect {
	public:
		[[nodiscard]] auto get_label(ptr<ir::node> node) const -> std::string_view override;

		void create_function(const ir::function_data_type& data_type);

		[[nodiscard]] auto create_local(u8 alignment, u8 size) -> ptr<ir::node>;
		[[nodiscard]] auto create_signed_integer(i64 value, u8 bit_width) -> ptr<ir::node>;

		void create_store(ptr<ir::node> destination, ptr<ir::node> value, u8 alignment);
		[[nodiscard]] auto create_load(ptr<ir::node> source, ir::data_type type, u8 alignment) -> ptr<ir::node>;
		void create_ret(const utility::memory<ptr<ir::node>, u8>& return_values);

		[[nodiscard]] auto create_projection(ir::data_type type, ptr<ir::node> source, u8 index) -> ptr<ir::node>;

		[[nodiscard]] auto append_memory(ptr<ir::node> new_memory) const->ptr<ir::node>;
		void append_input(ptr<ir::node> target, ptr<ir::node> input);
		[[nodiscard]] static auto get_parent_region(ptr<ir::node> node) -> ptr<ir::node>;

		// TODO: temp id
		// types
		static constexpr ir::data_type I32_TYPE = ir::data_type(100, 32);
	};
} // namespace baremetal
