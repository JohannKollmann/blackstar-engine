#include "IceSpline.h"
#include "IceLeakWatch.h"

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
		if(vPoints.size()<2)
			return;
		
		if(bClosed)
			vPoints.push_back(vPoints[0]);

		m_Sectors.clear();
		double* ppfCoordinates[3]={ICE_NEW double[vPoints.size()], ICE_NEW double[vPoints.size()], ICE_NEW double[vPoints.size()]};

		for(int iPoint=0; iPoint<(int)vPoints.size(); iPoint++)
		{
			ppfCoordinates[0][iPoint]=vPoints[iPoint].x;
			ppfCoordinates[1][iPoint]=vPoints[iPoint].y;
			ppfCoordinates[2][iPoint]=vPoints[iPoint].z;
		}

		int iMatrixSize=(vPoints.size()-1)*3;

		double** ppfMatrix=ICE_NEW double*[iMatrixSize+1];
		for(int iCol=0; iCol<iMatrixSize+1; iCol++)
			ppfMatrix[iCol]=ICE_NEW double[iMatrixSize];

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
			if(!bClosed)
			{
				ppfMatrix[1][iMatrixSize-1]=2.0;
			
				ppfMatrix[iMatrixSize-2][iMatrixSize-2]=2.0;
				ppfMatrix[iMatrixSize-3][iMatrixSize-2]=6.0;
			}
			else
			{
				//acceleration
				ppfMatrix[1][iMatrixSize-1]=-2.0;
				ppfMatrix[iMatrixSize-2][iMatrixSize-1]=2.0;
				ppfMatrix[iMatrixSize-3][iMatrixSize-1]=6.0;
				//slope
				ppfMatrix[2][iMatrixSize-2]=-1.0;
				ppfMatrix[iMatrixSize-1][iMatrixSize-2]=1.0;
				ppfMatrix[iMatrixSize-2][iMatrixSize-2]=2.0;
				ppfMatrix[iMatrixSize-3][iMatrixSize-2]=3.0;
			}
			
			ppfResults[iCoordinate]=ICE_NEW double[iMatrixSize];
			SolveLinearSystem(ppfMatrix, ppfResults[iCoordinate], iMatrixSize);
		}
		for(int iCol=0; iCol<iMatrixSize+1; iCol++)
			ICE_DELETE ppfMatrix[iCol];
		ICE_DELETE ppfMatrix;

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
		ICE_DELETE ppfCoordinates[0];
		ICE_DELETE ppfCoordinates[1];
		ICE_DELETE ppfCoordinates[2];
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
		
		if(bClosed)
			vPoints.push_back(vPoints[0]);

		m_Sectors.clear();
		m_SectorLengths.clear();
		double* ppfCoordinates[3]={ICE_NEW double[vPoints.size()], ICE_NEW double[vPoints.size()], ICE_NEW double[vPoints.size()]};

		for(int iPoint=0; iPoint<(int)vPoints.size(); iPoint++)
		{
			ppfCoordinates[0][iPoint]=vPoints[iPoint].x;
			ppfCoordinates[1][iPoint]=vPoints[iPoint].y;
			ppfCoordinates[2][iPoint]=vPoints[iPoint].z;
		}

		int iMatrixSize=(vPoints.size()-1)*3;

		double** ppfMatrix=ICE_NEW double*[iMatrixSize+1];
		for(int iCol=0; iCol<iMatrixSize+1; iCol++)
			ppfMatrix[iCol]=ICE_NEW double[iMatrixSize];

		double* ppfResults[3];

		for(int iCoordinate=0; iCoordinate<3; iCoordinate++)
		{
			for(int iCol=0; iCol<iMatrixSize+1; iCol++)
				for(int iLine=0; iLine<iMatrixSize; iLine++)
					ppfMatrix[iCol][iLine]=0.0;

			//this line is the equation stating that the spline sectors end is the next point
			double td=vPoints[0].w;
			ppfMatrix[0][0]=td*td*td;
			ppfMatrix[1][0]=td*td;
			ppfMatrix[2][0]=td;
			ppfMatrix[iMatrixSize][0]=ppfCoordinates[iCoordinate][1]-ppfCoordinates[iCoordinate][0];
			for(int iPoint=0; iPoint<(int)vPoints.size()-2; iPoint++)
			{
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
				td=vPoints[iPoint+1].w;
				ppfMatrix[iPoint*3+3][iPoint*3+3]=td*td*td;
				ppfMatrix[iPoint*3+4][iPoint*3+3]=td*td;
				ppfMatrix[iPoint*3+5][iPoint*3+3]=td;
				ppfMatrix[iMatrixSize][iPoint*3+3]=ppfCoordinates[iCoordinate][iPoint+2]-ppfCoordinates[iCoordinate][iPoint+1];
			}
			//the last two equations say that the slope in the beginning and end of the spline must be 0
			td=(vPoints[vPoints.size()-2].w);
			if(!bClosed)
			{
				ppfMatrix[1][iMatrixSize-1]=2.0;
			
				ppfMatrix[iMatrixSize-2][iMatrixSize-2]=2.0;
				ppfMatrix[iMatrixSize-3][iMatrixSize-2]=6.0*td;
			}
			else
			{
				//acceleration
				ppfMatrix[1][iMatrixSize-1]=-2.0;
				ppfMatrix[iMatrixSize-2][iMatrixSize-1]=2.0;
				ppfMatrix[iMatrixSize-3][iMatrixSize-1]=6.0*td;
				//slope
				ppfMatrix[2][iMatrixSize-2]=-1.0;
				ppfMatrix[iMatrixSize-1][iMatrixSize-2]=1.0;
				ppfMatrix[iMatrixSize-2][iMatrixSize-2]=2.0*td;
				ppfMatrix[iMatrixSize-3][iMatrixSize-2]=3.0*td*td;
			}
			
			ppfResults[iCoordinate]=ICE_NEW double[iMatrixSize];
			SolveLinearSystem(ppfMatrix, ppfResults[iCoordinate], iMatrixSize);
		}

		for(int iCol=0; iCol<iMatrixSize+1; iCol++)
			ICE_DELETE ppfMatrix[iCol];
		ICE_DELETE ppfMatrix;

		CSplineSector sector;
		double fTime=0.0;
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
			m_SectorLengths.push_back(fTime);
			fTime+=vPoints[iSector].w;
		}
		m_SectorLengths.push_back(fTime);

		ICE_DELETE ppfCoordinates[0];
		ICE_DELETE ppfCoordinates[1];
		ICE_DELETE ppfCoordinates[2];

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
				if(fMultiplier!=0.0 && fMultiplier!=-0.0)
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
	Spline::Sample(double fPos, int* piCurrentKey)
	{
		if(!m_bIsTimedSpline)
		{
			int iSector=(int)fPos;
			if((fPos-1.0)<(double)m_Sectors.size() && fPos>=0.0)
			{
				if(piCurrentKey)
					*piCurrentKey=iSector;
				return m_Sectors[iSector].CalcSample(fPos-(double)iSector);
			}
			if(piCurrentKey)
				*piCurrentKey=iSector;
			return Ogre::Vector3(0,0,0);
		}
		else
		{
			//search which sector we are in
			/*int iSearchPos=m_Sectors.size()/2;
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
			if(iSearchPos<(int)(m_SectorLengths.size()-1))
				if(fPos > m_SectorLengths[iSearchPos+1])
					iSearchPos++;
			//now check the position relative to the begin of the sector
			

			//*/
			int iSearchPos=m_SectorLengths.size()>>1;
			int iStep=(iSearchPos>>1)+1;

			while(iStep>0)
			{
				if(fPos<m_SectorLengths[iSearchPos])
					iSearchPos-=iStep;
				else if(fPos>m_SectorLengths[iSearchPos+1])
					iSearchPos+=iStep;
				else
					break;

				if(iSearchPos<0)
					iSearchPos=0;
				if(iSearchPos>=(int)m_SectorLengths.size())
					iSearchPos=(int)m_SectorLengths.size()-1;
				
				iStep>>=1;
			}

			if(fPos<m_SectorLengths[iSearchPos] && iSearchPos)
				iSearchPos--;
			if(iSearchPos<(int)m_SectorLengths.size()-1)
				if(fPos>m_SectorLengths[iSearchPos+1])
					iSearchPos++;

			if(piCurrentKey)
				*piCurrentKey=iSearchPos;

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
