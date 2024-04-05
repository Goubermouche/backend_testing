// types utility header
#pragma once

#include <unordered_map>
#include <unordered_set>
#include <type_traits>
#include <functional>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <format>
#include <bitset>
#include <array>

namespace utility {
	namespace types {
		// signed integers
		using i8 = int8_t;
		using i16 = int16_t;
		using i32 = int32_t;
		using i64 = int64_t;

		// unsigned integers
		using u8 = uint8_t;
		using u16 = uint16_t;
		using u32 = uint32_t;
		using u64 = uint64_t;

		// floating point
		using f32 = float;
		using f64 = double;
	} // namespace utility::types

	using namespace types;
} // namespace utility
