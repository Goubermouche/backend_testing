#pragma once
#include "baremetal/intermediate_representation/dialect.h"

namespace baremetal {
	namespace detail {
		template<typename... types>
		struct dialect_list_derive {};

		template<typename type, typename... types>
		struct dialect_list_derive<type, types...> : type, dialect_list_derive<types...> {
			dialect_list_derive() : dialect_list_derive<types...>() {}
		};

		template<typename type>
		struct dialect_list_derive<type> : type {
			dialect_list_derive() = default;
		};
	} // namespace detail

	template<derived_from_dialect_base... dialects>
	class module : public virtual module_data, public detail::dialect_list_derive<dialects...> {
	public:
		module() {
			((m_dialect_pointers[utility::get_type_id<dialects>()] = static_cast<dialects*>(this)), ...);
		}

		virtual ~module() = default;
	};
} // namespace baremetal
