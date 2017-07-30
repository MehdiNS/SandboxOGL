#pragma once
#include <cstdint>

namespace OGL
{
	using s8 = char; // GL seems to whine otherwise
	using s16 = int16_t;
	using s32 = int32_t;
	using s64 = int64_t;

	using u8 = unsigned char;
	using u16 = uint16_t;
	using u32 = uint32_t;
	using u64 = uint64_t;

	using f32 = float;
	using f64 = double;
}
