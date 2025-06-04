#include "FPSCounter.h"

#include <limits>
#include <algorithm>

namespace VulkanCore {

	FPSCounter::FPSCounter()
        : calcMinInterval({ 0 })
        , lastCalcTime({ 0 })
        , frameCount(0.0f)
        , FPS(0.0f)
        , minFPS(std::numeric_limits<float>::max())
        , maxFPS(std::numeric_limits<float>::min())
        , avgFPS(0.0f)
	{
        calcMinInterval = MillisecondsToTime(CALC_MIN_INTERVAL_MILLISECONDS);
	}

    FPSCounter& FPSCounter::GetInstance()
    {
        static FPSCounter instance;
        return instance;
    }

    void FPSCounter::Start(const TimeData& appTime)
    {
        lastCalcTime = appTime.m_Time;
        frameCount = 0.0f;
        FPS = 0.0f;
    }

    void FPSCounter::NewFrame(const TimeData& appTime)
    {
        frameCount += 1.0f;
        if (appTime.m_Time >= lastCalcTime + calcMinInterval)
        {
            FPS = frameCount / TimeToSeconds<float>(appTime.m_Time - lastCalcTime);
            lastCalcTime = appTime.m_Time;
            frameCount = 0.0f;

            minFPS = std::min(minFPS, FPS);
            maxFPS = std::max(maxFPS, FPS);
        }
    }

    void FPSCounter::Reset()
    {
        frameCount = 0.0f;
        minFPS = std::numeric_limits<float>::max();
        maxFPS = std::numeric_limits<float>::min();
    }

} // namespace VulkanCore
