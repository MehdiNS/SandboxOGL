#pragma once
#include "Gui.h"

namespace OGL
{
	class Timer
	{
	public:
		using value_type = std::chrono::time_point<std::chrono::system_clock>;
		Timer(f32& value);
		~Timer() noexcept;
	
	private:
		value_type m_beforeCPU;
		f32& m_delta; // in ms
		u32 m_query[2];
		ProfileType m_profileType; // used so that we profile the same thing at the construction/destruction of the timer
	};
}