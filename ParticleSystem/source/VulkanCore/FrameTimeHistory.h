#pragma once

#include <vector>

namespace VulkanCore {

    class FrameTimeHistory
    {
    public:
        struct Entry
        {
            float deltaTime;
            float log2DeltaTime;
        };

        // Constructor
        FrameTimeHistory();

        // Destructor
        ~FrameTimeHistory();

        // Not copyable
        FrameTimeHistory(const FrameTimeHistory&) = delete;
        FrameTimeHistory& operator = (const FrameTimeHistory&) = delete;

        // Not moveable
        FrameTimeHistory(FrameTimeHistory&&) = delete;
        FrameTimeHistory& operator = (FrameTimeHistory&&) = delete;

        // Getters
        static FrameTimeHistory& GetInstance();
        inline size_t GetCount() const { return count; }
        Entry GetEntry(size_t i) const;

        void Reset();
        void Post(float deltaTime);

    private:
        static constexpr size_t CAPACITY = 1024;

        size_t back;
        size_t front;
        size_t count;
        std::vector<Entry> entries;
    };

} // namespace VulkanCore
