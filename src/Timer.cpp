#include "stdafx.h"
#include "Timer.h"
#include "Util.h"

namespace OGL
{
	Timer::Timer(f32& value)
		: m_delta{ value }
	{
		m_profileType = s_profileType;
		if (m_profileType == CPU_PROFILE)
		{
			m_beforeCPU = std::chrono::system_clock::now();
		}
		else
		{
			glGenQueries(2, m_query);
			glQueryCounter(m_query[0], GL_TIMESTAMP);
		}
	}

	Timer::~Timer() noexcept
	{
		f32 delta;
		
		if (m_profileType == CPU_PROFILE)
		{
			auto afterCPU = std::chrono::system_clock::now();
			delta = 0.001f * std::chrono::duration_cast<std::chrono::microseconds>(afterCPU - m_beforeCPU).count();
		}
		else
		{
			glQueryCounter(m_query[1], GL_TIMESTAMP);
		
			s32 stopTimerAvailable = 0;
			while (!stopTimerAvailable)
				glGetQueryObjectiv(m_query[1], GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);
		
			u64 startTime, stopTime;
			glGetQueryObjectui64v(m_query[0], GL_QUERY_RESULT, &startTime);
			glGetQueryObjectui64v(m_query[1], GL_QUERY_RESULT, &stopTime);
			delta = 0.001f * 0.001f * (stopTime - startTime);
		}
		
		m_delta = delta;
	}
}