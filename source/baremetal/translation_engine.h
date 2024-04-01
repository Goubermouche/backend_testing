#pragma once
#include "target.h"
#include "pass.h"

#include <utility/memory/memory.h>
#include <utility/managed_ptr.h>

namespace baremetal {
	using namespace utility::types;

	class translation_engine {
	public:
		void translate(target& target, module_data& module);

		void add_pass(managed_ptr<pass> pass);
	private:
		utility::memory<managed_ptr<pass>> m_passes;
	};
} // namespace baremetal
