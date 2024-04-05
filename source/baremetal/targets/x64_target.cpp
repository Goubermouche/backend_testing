#include "x64_target.h"

#include "baremetal/dialects/core_dialect.h"
#include "baremetal/dialects/gpu_dialect.h"

namespace baremetal {
	x64_target::x64_target(context& context) : target(context) {
		m_isel_functions.resize(context.m_index_map.size());

		m_isel_functions[context.get_dialect_index<core_dialect>()] = { isel_core };
		m_isel_functions[context.get_dialect_index<gpu_dialect>()]  = { isel_gpu };
	}
} // namespace baremetal
