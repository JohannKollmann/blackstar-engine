#include <vector>
#include <Ogre.h>

namespace Ice
{

class Spline
{
public:
	Spline(){m_Sectors.clear();}
	Spline(std::vector<Ogre::Vector3> vPoints);
	void SetPoints(std::vector<Ogre::Vector3> vPoints);
	Ogre::Vector3 Sample(double fPos);
	double GetLength();

	class CSplineSector
	{
	public:
		Ogre::Vector3 CalcSample(double fPos);//pos in [0:1]
		double m_aafParams[3][4];
	};
private:
	void SolveLinearSystem(double** ppfMatrix, double* ppfResults, int iMatrixSize);//takes matrices in n+1 x n form
	std::vector<CSplineSector> m_Sectors;
};

};