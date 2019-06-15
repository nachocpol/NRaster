#pragma once

#include <chrono>

#define TIME_STAMP std::chrono::time_point<std::chrono::system_clock>
#define NProfilerGet() NProfiler::Instance()

class NProfiler
{
private:
	NProfiler(float cpuGHz);
	NProfiler(const NProfiler& other);
	~NProfiler();

public:
	static NProfiler* Instance();

	TIME_STAMP Now();
	float TimeDiffMS(const TIME_STAMP& start, const TIME_STAMP& end);
	uint32_t TimeMSToCycles(float elapsedMS);

private:
	float m_cpuGHz;

};