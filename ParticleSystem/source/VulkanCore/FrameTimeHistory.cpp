#include "FrameTimeHistory.h"

#include <glm/glm.hpp>

namespace VulkanCore {

	FrameTimeHistory::FrameTimeHistory()
		: back(0)
		, front(0)
		, count(0)
		, entries(CAPACITY)
	{

	}

	FrameTimeHistory::~FrameTimeHistory()
	{

	}

	FrameTimeHistory& FrameTimeHistory::GetInstance()
	{
		static FrameTimeHistory instance;
		return instance;
	}

	FrameTimeHistory::Entry FrameTimeHistory::GetEntry(size_t i) const
	{
		i = (back + count - i - 1) % CAPACITY;
		return entries[i];
	}

	void FrameTimeHistory::Reset()
	{
		back = 0;
		front = 0;
		count = 0;
	}

	void FrameTimeHistory::Post(float deltaTime)
	{
		entries[front] = { deltaTime, glm::log2(deltaTime) };
		front = (front + 1) % CAPACITY;

		if (count == CAPACITY)
		{
			back = front;
		}
		else
		{
			++count;
		}
	}

} // namespace VulkanCore
