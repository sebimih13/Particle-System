#include "Time.h"

#include <algorithm>
#include <chrono>

namespace VulkanCore {

	Time Time::Now()
	{
        auto now = std::chrono::steady_clock::now();
        return { duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count() };
	}

    //////////////////////////////
    // TimeToSeconds
    //////////////////////////////
    template<typename T>
    T TimeToSeconds(Time t)
    {
        return std::chrono::duration<T>(std::chrono::nanoseconds(t.value)).count();
    }
    template float TimeToSeconds<float>(Time t);
    template double TimeToSeconds<double>(Time t);

    //////////////////////////////
    // SecondsToTime
    //////////////////////////////
    template<typename T>
    Time SecondsToTime(T sec)
    {
        auto duration = std::chrono::duration<double>(sec);
        auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);
        return { nanoseconds.count() };
    }
    template Time SecondsToTime<float>(float sec);
    template Time SecondsToTime<double>(double sec);

    //////////////////////////////
    // TimeToMilliseconds
    //////////////////////////////
    template<typename T>
    T TimeToMilliseconds(Time t)
    {
        return static_cast<T>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::nanoseconds(t.value)).count());
    }

    //////////////////////////////
    // MillisecondsToTime
    //////////////////////////////
    template<typename T>
    Time MillisecondsToTime(T ms)
    {
        return { std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<T, std::milli>(ms)).count() };
    }
    template Time MillisecondsToTime<unsigned int>(unsigned int);

    void TimeData::Start(Time now)
    {
        *this = {};
        absoluteStartTime = now;
    }

    void TimeData::NewFrameFromNow(Time now)
    {
        // Cannot be smaller than previous time
        Time newTime = std::max(now - absoluteStartTime, previousTime);
        deltaTime = newTime - time;
        previousTime = time;
        time = newTime;
        timeFloat = TimeToSeconds<float>(newTime);
        deltaTimeFloat = TimeToSeconds<float>(deltaTime);
        ++frameIndex;
    }

    void TimeData::NewFrameFromDelta(Time delta)
    {
        // Cannot be smaller than previous time
        delta = std::max(delta, Time{ 0 });
        Time newTime = time + delta;
        deltaTime = delta;
        previousTime = time;
        time = newTime;
        timeFloat = TimeToSeconds<float>(newTime);
        deltaTimeFloat = TimeToSeconds<float>(deltaTime);
        ++frameIndex;
    }

    float SecondsToMiliseconds(float sec)
    {
        return sec * 1000.0f;
    }

} // namespace VulkanCore
