#pragma once

#include <utility>

#define SPATIAL_COVERAGE_DATATYPE unsigned long

class SpatialCoverage
{
public:
	SpatialCoverage();
	std::pair<float, float> getNext();
private:
	SPATIAL_COVERAGE_DATATYPE m_iCounter;
};
