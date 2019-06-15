#include "NProfiler.h"
#include <Windows.h> // Sleep
#include <iostream>

NProfiler::NProfiler(float cpuGHz):
	m_cpuGHz(cpuGHz)
{
}

NProfiler::NProfiler(const NProfiler& other)
{
}

NProfiler::~NProfiler()
{
}

NProfiler* NProfiler::Instance()
{
	float cpuGHz = 3.8f; // Find a way of retireving this, maybe registry?

	static NProfiler* kInstance = nullptr;
	if (!kInstance)
	{
		kInstance = new NProfiler(cpuGHz);

		// Simple test to make sure this works!
		{
			auto ts = NProfilerGet()->Now();
			Sleep(1000);
			auto te = NProfilerGet()->Now();
			float ms = NProfilerGet()->TimeDiffMS(ts, te);
			uint32_t cycles = NProfilerGet()->TimeMSToCycles(ms);
			std::cout << "Numer of cycles per second:" << cycles << std::endl;
		}

	}
	return kInstance;
}

TIME_STAMP NProfiler::Now()
{
	return std::chrono::system_clock::now();
}

float NProfiler::TimeDiffMS(const TIME_STAMP& start, const TIME_STAMP& end)
{
	std::chrono::duration<double> diff = end - start;
	return diff.count() * 1000.0f;
}

uint32_t NProfiler::TimeMSToCycles(float elapsedMS)
{
	// CPU GHz --> cicles per seconds --> cicles per milisecond
	const uint32_t ciclesPerMs = (uint32_t)((m_cpuGHz * 1000000000.0f) / 1000.0f);
	return elapsedMS * ciclesPerMs;
}

