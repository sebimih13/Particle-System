#include "FPSCounter.h"

namespace VulkanCore {

	FPSCounter::FPSCounter()
	{
        m_CalcMinInterval = MillisecondsToTime(CALC_MIN_INTERVAL_MILLISECONDS);
	}

    FPSCounter& FPSCounter::GetInstance()
    {
        static FPSCounter instance;
        return instance;
    }

    void FPSCounter::Start(const TimeData& appTime)
    {
        m_LastCalcTime = appTime.m_Time;
        m_FrameCount = 0.f;
        m_FPS = 0.f;
    }

    void FPSCounter::NewFrame(const TimeData& appTime)
    {
        m_FrameCount += 1.f;
        if (appTime.m_Time >= m_LastCalcTime + m_CalcMinInterval)
        {
            m_FPS = m_FrameCount / TimeToSeconds<float>(appTime.m_Time - m_LastCalcTime);
            m_LastCalcTime = appTime.m_Time;
            m_FrameCount = 0.f;
        }
    }

} // namespace VulkanCore
