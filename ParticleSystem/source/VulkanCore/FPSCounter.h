#pragma once

#include "Time.h"

namespace VulkanCore {

    class FPSCounter
    {
    public:
        // Constructor
        FPSCounter();

        static FPSCounter& GetInstance();

        void Start(const TimeData& appTime);
        void NewFrame(const TimeData& appTime);
        void Reset();

        // Getters
        float GetFPS() const { return FPS; }
        float GetMinFPS() const { return minFPS; }
        float GetMaxFPS() const { return maxFPS; }
        float GetAvgFPS() const { return avgFPS; }

    private:
        static constexpr uint32_t CALC_MIN_INTERVAL_MILLISECONDS = 500;

        Time calcMinInterval;
        Time lastCalcTime;
        float frameCount;
        float FPS;
        float minFPS;
        float maxFPS;
        float avgFPS;
    };

} // namespace VulkanCore
