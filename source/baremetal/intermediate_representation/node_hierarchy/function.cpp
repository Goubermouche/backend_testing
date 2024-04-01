#include "function.h"

namespace baremetal::ir {
	function::function(const function_data_type& data_type)
		: block_allocator(1024), m_data_type(data_type), m_node_count(0) {}

	function::~function() {}
} // namespace baremetal::ir 

