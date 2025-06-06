#pragma once

#include <cstdint>

namespace VulkanCore {

    struct Time
    {
        int64_t value = 0;

        static Time Now();

        bool IsZero() const { return value == 0; }
        Time operator+(Time rhs) const { return { value + rhs.value }; }
        Time operator-(Time rhs) const { return { value - rhs.value }; }
        Time& operator+=(Time rhs) { value += rhs.value; return *this; }
        Time& operator-=(Time rhs) { value -= rhs.value; return *this; }
        bool operator==(Time rhs) const { return value == rhs.value; }
        bool operator!=(Time rhs) const { return value != rhs.value; }
        bool operator<(Time rhs) const { return value < rhs.value; }
        bool operator>(Time rhs) const { return value > rhs.value; }
        bool operator<=(Time rhs) const { return value <= rhs.value; }
        bool operator>=(Time rhs) const { return value >= rhs.value; }
    };

    // Use T = float, double
    template<typename T>
    T TimeToSeconds(Time t);

    template<typename T>
    Time SecondsToTime(T sec);

    // Use T = float, double, int32_t, int64_t, uint32_t, uint64_t
    template<typename T>
    T TimeToMilliseconds(Time t);

    template<typename T>
    Time MillisecondsToTime(T ms);

    float SecondsToMiliseconds(float sec);

    // absoluteStartTime = fetched using Now() or 0 if uninitialized. Every other member is relative to this one
    // Time = time since AbsoluteStartTime
    // DeltaTime = time since previous frame
    struct TimeData
    {
        Time absoluteStartTime = { 0 };
        Time previousTime = { 0 };
        Time time = { 0 };
        Time deltaTime = { 0 };
        float timeFloat = 0.f;
        float deltaTimeFloat = 0.f;
        uint32_t frameIndex = 0;

        void Start(Time now);
        void NewFrameFromNow(Time now);
        void NewFrameFromDelta(Time delta);
    };

} // namespace VulkanCore
