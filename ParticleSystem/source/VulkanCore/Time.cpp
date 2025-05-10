#include "Time.h"

#include <algorithm>
#include <chrono>

namespace VulkanCore {

	Time Now()
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
        return std::chrono::duration<T>(std::chrono::nanoseconds(t.m_Value)).count();
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
        return static_cast<T>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::nanoseconds(t.m_Value)).count());
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
        m_AbsoluteStartTime = now;
    }

    void TimeData::NewFrameFromNow(Time now)
    {
        // Cannot be smaller than previous time
        Time newTime = std::max(now - m_AbsoluteStartTime, m_PreviousTime);
        m_DeltaTime = newTime - m_Time;
        m_PreviousTime = m_Time;
        m_Time = newTime;
        m_Time_Float = TimeToSeconds<float>(newTime);
        m_DeltaTime_Float = TimeToSeconds<float>(m_DeltaTime);
        ++m_FrameIndex;
    }

    void TimeData::NewFrameFromDelta(Time delta)
    {
        // Cannot be smaller than previous time
        delta = std::max(delta, Time{ 0 });
        Time newTime = m_Time + delta;
        m_DeltaTime = delta;
        m_PreviousTime = m_Time;
        m_Time = newTime;
        m_Time_Float = TimeToSeconds<float>(newTime);
        m_DeltaTime_Float = TimeToSeconds<float>(m_DeltaTime);
        ++m_FrameIndex;
    }

} // namespace VulkanCore
