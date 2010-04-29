#include "IceSpline.h"

#include <stdio.h>
#include <vector>

namespace Ice
{

	Spline::Spline(std::vector<Ogre::Vector3> vPoints, bool bClosed=false)
	{
		SetPoints(vPoints, bClosed);
	}
	void
	Spline::SetPoints(std::vector<Ogre::Vector3> vPoints, bool bClosed)
	{
		m_Sectors.clear();
		double* ppfCoordinates[3]={new double[vPoints.size()], new double[vPoints.size()], new double[vPoints.size()]};

		for(int iPoint=0; iPoint<(int)vPoints.size(); iPoint++)
		{
			ppfCoordinates[0][iPoint]=vPoints[iPoint].x;
			ppfCoordinates[1][iPoint]=vPoints[iPoint].y;
			ppfCoordinates[2][iPoint]=vPoints[iPoint].z;
		}

		int iMatrixSize=(vPoints.size()-1)*3;

		double** ppfMatrix=new double*[iMatrixSize+1];
		for(int iCol=0; iCol<iMatrixSize+1; iCol++)
			ppfMatrix[iCol]=new double[iMatrixSize];

		double* ppfResults[3];

		for(int iCoordinate=0; iCoordinate<3; iCoordinate++)
		{
			for(int iCol=0; iCol<iMatrixSize+1; iCol++)
				for(int iLine=0; iLine<iMatrixSize; iLine++)
					ppfMatrix[iCol][iLine]=0.0;

			ppfMatrix[0][0]=1.0;
			ppfMatrix[1][0]=1.0;
			ppfMatrix[2][0]=1.0;
			ppfMatrix[iMatrixSize][0]=ppfCoordinates[iCoordinate][1]-ppfCoordinates[iCoordinate][0];
			for(int iPoint=0; iPoint<(int)vPoints.size()-2; iPoint++)
			{
				//this line states that the slope at the end of the sector is equal to the slope in the beginning of the next sector
				ppfMatrix[iPoint*3][iPoint*3+1]=3.0;
				ppfMatrix[iPoint*3+1][iPoint*3+1]=2.0;
				ppfMatrix[iPoint*3+2][iPoint*3+1]=1.0;
				ppfMatrix[iPoint*3+5][iPoint*3+1]=-1.0;

				ppfMatrix[iPoint*3][iPoint*3+2]=6.0;
				ppfMatrix[iPoint*3+1][iPoint*3+2]=2.0;
				ppfMatrix[iPoint*3+4][iPoint*3+2]=-1.0;
				//this line is the equation stating that the spline sectors end is the next point
				ppfMatrix[iPoint*3+3][iPoint*3+3]=1.0;
				ppfMatrix[iPoint*3+4][iPoint*3+3]=1.0;
				ppfMatrix[iPoint*3+5][iPoint*3+3]=1.0;
				ppfMatrix[iMatrixSize][iPoint*3+3]=ppfCoordinates[iCoordinate][iPoint+2]-ppfCoordinates[iCoordinate][iPoint+1];
			}
			//the last two equations say that the slope in the beginning and end of the spline must be 0
			ppfMatrix[1][iMatrixSize-1]=2.0;
			//
			ppfMatrix[iMatrixSize-2][iMatrixSize-2]=2.0;
			ppfMatrix[iMatrixSize-3][iMatrixSize-2]=6.0;

			ppfResults[iCoordinate]=new double[iMatrixSize];
			SolveLinearSystem(ppfMatrix, ppfResults[iCoordinate], iMatrixSize);
		}
		CSplineSector sector;
		for(int iSector=0; iSector<(int)vPoints.size()-1; iSector++)
		{
			for(int iCoordinate=0; iCoordinate<3; iCoordinate++)
			{
				for(int iParam=0; iParam<3; iParam++)
				{
					sector.m_aafParams[iCoordinate][iParam]=ppfResults[iCoordinate][iParam+iSector*3];
				}
				sector.m_aafParams[iCoordinate][3]=ppfCoordinates[iCoordinate][iSector];
			}
			m_Sectors.push_back(sector);
		}
		m_bIsTimedSpline=false;
	}

	bool CompVectorByW(Ogre::Vector4 v1, Ogre::Vector4 v2) {return v1.w<v2.w;}

	Spline::Spline(std::vector<Ogre::Vector4> vPoints, bool bClosed)
	{
		SetPoints(vPoints, bClosed);
	}
	void
	Spline::SetPoints(std::vector<Ogre::Vector4> vPoints, bool bClosed)
	{
		if(vPoints.size()<2)
			return;
		std::sort(vPoints.begin(), vPoints.end(), CompVectorByW);		

		m_Sectors.clear();
		m_SectorLengths.clear();
		double* ppfCoordinates[3]={new double[vPoints.size()], new double[vPoints.size()], new double[vPoints.size()]};

		for(int iPoint=0; iPoint<(int)vPoints.size(); iPoint++)
		{
			ppfCoordinates[0][iPoint]=vPoints[iPoint].x;
			ppfCoordinates[1][iPoint]=vPoints[iPoint].y;
			ppfCoordinates[2][iPoint]=vPoints[iPoint].z;
		}

		int iMatrixSize=(vPoints.size()-1)*3;

		double** ppfMatrix=new double*[iMatrixSize+1];
		for(int iCol=0; iCol<iMatrixSize+1; iCol++)
			ppfMatrix[iCol]=new double[iMatrixSize];

		double* ppfResults[3];

		for(int iCoordinate=0; iCoordinate<3; iCoordinate++)
		{
			for(int iCol=0; iCol<iMatrixSize+1; iCol++)
				for(int iLine=0; iLine<iMatrixSize; iLine++)
					ppfMatrix[iCol][iLine]=0.0;

			//this line is the equation stating that the spline sectors end is the next point
			double td=vPoints[1].w-vPoints[0].w;
			ppfMatrix[0][0]=td*td*td;
			ppfMatrix[1][0]=td*td;
			ppfMatrix[2][0]=td;
			ppfMatrix[iMatrixSize][0]=ppfCoordinates[iCoordinate][1]-ppfCoordinates[iCoordinate][0];
			for(int iPoint=0; iPoint<(int)vPoints.size()-2; iPoint++)
			{
				//td=vPoints[iPoint+2].w-vPoints[iPoint+1].w;
				td=vPoints[iPoint+1].w-vPoints[iPoint].w;
				//this line states that the slope at the end of the sector is equal to the slope in the beginning of the next sector
				ppfMatrix[iPoint*3][iPoint*3+1]=3.0*td*td;
				ppfMatrix[iPoint*3+1][iPoint*3+1]=2.0*td;
				ppfMatrix[iPoint*3+2][iPoint*3+1]=1.0;
				ppfMatrix[iPoint*3+5][iPoint*3+1]=-1.0;
				//this line states that the acceleration at the end of the sector is equal to the acceleration in the beginning of the next sector
				ppfMatrix[iPoint*3][iPoint*3+2]=6.0*td;
				ppfMatrix[iPoint*3+1][iPoint*3+2]=2.0;
				ppfMatrix[iPoint*3+4][iPoint*3+2]=-2.0;
				//this line is the equation stating that the spline sectors end is the next point
				td=vPoints[iPoint+2].w-vPoints[iPoint+1].w;
				ppfMatrix[iPoint*3+3][iPoint*3+3]=td*td*td;
				ppfMatrix[iPoint*3+4][iPoint*3+3]=td*td;
				ppfMatrix[iPoint*3+5][iPoint*3+3]=td;
				ppfMatrix[iMatrixSize][iPoint*3+3]=ppfCoordinates[iCoordinate][iPoint+2]-ppfCoordinates[iCoordinate][iPoint+1];
			}
			//the last two equations say that the slope in the beginning and end of the spline must be 0
			ppfMatrix[0][iMatrixSize-1]=6.0;
			ppfMatrix[1][iMatrixSize-1]=2.0;
			//
			ppfMatrix[iMatrixSize-2][iMatrixSize-2]=2.0;
			ppfMatrix[iMatrixSize-3][iMatrixSize-2]=6.0;

			ppfResults[iCoordinate]=new double[iMatrixSize];
			SolveLinearSystem(ppfMatrix, ppfResults[iCoordinate], iMatrixSize);
		}
		CSplineSector sector;
		for(int iSector=0; iSector<(int)vPoints.size()-1; iSector++)
		{
			for(int iCoordinate=0; iCoordinate<3; iCoordinate++)
			{
				for(int iParam=0; iParam<3; iParam++)
				{
					sector.m_aafParams[iCoordinate][iParam]=ppfResults[iCoordinate][iParam+iSector*3];
				}
				sector.m_aafParams[iCoordinate][3]=ppfCoordinates[iCoordinate][iSector];
			}
			m_Sectors.push_back(sector);
			m_SectorLengths.push_back(vPoints[iSector].w);

		}
		m_SectorLengths.push_back(vPoints[vPoints.size()-1].w);
		m_bIsTimedSpline=true;
	}

	void
	Spline::SolveLinearSystem(double** ppfMatrix, double* ppfResults, int iMatrixSize)
	{
		//Solve the matrix
		for(int iStartLine=0; iStartLine<iMatrixSize; iStartLine++)
		{
			for(int iCurrLine=iStartLine+1; iCurrLine<iMatrixSize; iCurrLine++)
			{
				double fMultiplier=-ppfMatrix[iStartLine][iCurrLine]/ppfMatrix[iStartLine][iStartLine];
				if(fMultiplier!=0 && fMultiplier!=-0)
				{
					for(int iCol=iStartLine; iCol<iMatrixSize+1; iCol++)
						ppfMatrix[iCol][iCurrLine]+=fMultiplier*ppfMatrix[iCol][iStartLine];
				}
			}
		}
		//now calc the results
		for(int iLine=0; iLine<iMatrixSize; iLine++)
		{
			int iMatrixLine=iMatrixSize-1-iLine;
			double fRightSide=ppfMatrix[iMatrixSize][iMatrixLine];//sums up the right side of the equation
			for(int iCol=iMatrixSize-1; iCol>iMatrixLine; iCol--)
				fRightSide-=ppfResults[iCol]*ppfMatrix[iCol][iMatrixLine];
			ppfResults[iMatrixLine]=fRightSide/ppfMatrix[iMatrixLine][iMatrixLine];
		}
	}

	Ogre::Vector3
	Spline::CSplineSector::CalcSample(double fPos)
	{
		double afResults[3]={0,0,0};
		double fPosPow=1.0;
		for(int iParam=3; iParam>=0; iParam--)
		{
			for(int iDimension=0; iDimension<3; iDimension++)
				afResults[iDimension]+=fPosPow*m_aafParams[iDimension][iParam];
			fPosPow*=fPos;
		}
		return Ogre::Vector3((Ogre::Real)afResults[0], (Ogre::Real)afResults[1], (Ogre::Real)afResults[2]);
	}

	Ogre::Vector3
	Spline::Sample(double fPos)
	{
		if(!m_bIsTimedSpline)
		{
			int iSector=(int)fPos;
			if((fPos-1.0)<(double)m_Sectors.size() && fPos>=0.0)
			{
				return m_Sectors[iSector].CalcSample(fPos-(double)iSector);
			}
			return Ogre::Vector3(0,0,0);
		}
		else
		{
			//search which sector we are in
			int iSearchPos=m_Sectors.size()/2;
			int iStep=iSearchPos+1;
			
			while(iStep>1)
			{
				iStep>>=1;
				double fSample=m_SectorLengths[iSearchPos];
				if(fPos<fSample)
					iSearchPos-=iStep;
				else
					iSearchPos+=iStep;
			}
			if(fPos < m_SectorLengths[iSearchPos] && iSearchPos)
				iSearchPos--;
			//now check the position relative to the begin of the sector
			return m_Sectors[iSearchPos].CalcSample(fPos-m_SectorLengths[iSearchPos]);
		}
	}


	double
	Spline::GetLength()
	{
		if(!m_bIsTimedSpline)
		{
			return (double)((int)m_Sectors.size());
		}
		else
		{
			return m_SectorLengths[m_Sectors.size()];
		}
	}

};
