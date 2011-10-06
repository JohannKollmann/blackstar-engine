
#include "IceSpatialCoverage.h"

SpatialCoverage::SpatialCoverage()
{
	m_iCounter=0;
}

std::pair<float, float>
SpatialCoverage::getNext()
{
	static const unsigned int nBits=(sizeof(SpatialCoverage::m_iCounter)*8);
	static const unsigned int nUsableBits=nBits/3;// (3/8)
	static const SPATIAL_COVERAGE_DATATYPE nMaxUsable (((SPATIAL_COVERAGE_DATATYPE)1)<<nUsableBits);

	SPATIAL_COVERAGE_DATATYPE x=0, y=0;
	for (unsigned int i = 0; i < nUsableBits; ++i)
	{
		unsigned int j=i<<1;

		SPATIAL_COVERAGE_DATATYPE xbits=m_iCounter&(((SPATIAL_COVERAGE_DATATYPE)1)<<(j));
		SPATIAL_COVERAGE_DATATYPE ybits=m_iCounter&(((SPATIAL_COVERAGE_DATATYPE)1)<<(j+1));

		x|=xbits<<(nBits-j-i-1);
		y|=((ybits>>1)^xbits)<<(nBits-j-i-1);
	}
	x>>=nBits-nUsableBits;
	y>>=nBits-nUsableBits;
	m_iCounter++;
	return std::make_pair((float)x/(float)nMaxUsable, (float)y/(float)nMaxUsable);
}
