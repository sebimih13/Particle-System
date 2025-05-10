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
        float GetFPS() const { return m_FPS; }

    private:
        static constexpr uint32_t CALC_MIN_INTERVAL_MILLISECONDS = 500;

        Time m_CalcMinInterval = { 0 };
        Time m_LastCalcTime = { 0 };
        float m_FrameCount = 0.0f;
        float m_FPS = 0.0f;
    };

} // namespace VulkanCore
