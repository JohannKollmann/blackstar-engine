#include "SGTFontTextures.h"
#include <fstream>

SGTFontTextures::SGTFontTextures(std::string strFontSpacings)
{
	m_aaiFontSpacings[255][1]=-1;

	std::ifstream spacingFile(strFontSpacings.c_str(), std::ios_base::ate);
	std::stringstream sstr;

	if(!spacingFile.good())
		return;

	//parse the file
	//first check file size
	int iFileSize=spacingFile.tellg();
	spacingFile.seekg(0, std::ios_base::beg);
	//reserve some mem and load it
	char* pcFile=new char[iFileSize+1];
	int iChar;
	for(iChar=iFileSize>>1; iChar<iFileSize+1; iChar++)
		pcFile[iChar]='\0';//set half the array to 0, since \r\n is parsed as \n
	spacingFile.read(pcFile, iFileSize);
	spacingFile.close();
	int iCurrWidth=0;
	//read units per tile
	iChar=0;
	while('0'<=pcFile[iChar] && pcFile[iChar]<='9')
	{
		sstr<<pcFile[iChar];
		iChar++;
	}
	m_TexelWidth=0;
	sstr>>m_TexelWidth;
	if(m_TexelWidth==0)
		return;
	do
	{
		iChar++;//jump over the delimiter
		if(iChar>iFileSize-1)
			break;

		sstr.clear();
		//read start
		int iStartChar=iChar;
		while('0'<=pcFile[iChar] && pcFile[iChar]<='9')
		{
			sstr<<pcFile[iChar];
			iChar++;
		}
		sstr>>m_aaiFontSpacings[iCurrWidth][0];
		if(iStartChar==iChar || m_aaiFontSpacings[iCurrWidth][0]<0 || m_aaiFontSpacings[iCurrWidth][0]>m_TexelWidth)
			break;
		iChar++;//jump over the delimiter
		if(iChar>iFileSize-1)
			break;

		sstr.clear();
		//read end
		iStartChar=iChar;
		while('0'<=pcFile[iChar] && pcFile[iChar]<='9')
		{
			sstr<<pcFile[iChar];
			iChar++;
		}
		sstr>>m_aaiFontSpacings[iCurrWidth][1];
		if(iStartChar==iChar || m_aaiFontSpacings[iCurrWidth][1]<0 || m_aaiFontSpacings[iCurrWidth][1]>m_TexelWidth)
			break;
		//increase position
		iCurrWidth++;
	}while(iChar<=iFileSize && iCurrWidth<256);

	if(iCurrWidth!=256)
		return;
	delete pcFile;
}

#define UNSIGN_CHAR(c) ((int)c>=0 ? (int)c : 256+(int)c)

Ogre::TexturePtr
SGTFontTextures::CreateTextTexture(std::string strText, int iMaxWidth, int iMaxHeight, int& iActualWidth, int& iActualHeight)
{
	if(m_aaiFontSpacings[255][1]==-1)
		return Ogre::TexturePtr();

	const int iTabWidth=4*(m_aaiFontSpacings[' '][1]-m_aaiFontSpacings[' '][0]);
	iActualHeight=iActualWidth=0;
	SCharInfo* pChars=new SCharInfo[strText.size()*2];//some reserves für line breaks
	int iCurrLinePos=0;
	unsigned short wCurrColor=0xf000;//ARGB
	unsigned int iChar=0;
	int nChars=0;
	for(; iChar<strText.size(); iChar++)
	{
		//treat the escape character '$'
		if(strText[iChar]=='$')
		{
			if(strText[iChar+1]=='$')
				iChar++;
			else
			{
				if(strText.size()-iChar>5)
				{
					bool bWellFormed=true;
					for(int iDigit=0; iDigit<4; iDigit++)
						if(!((strText[iChar+1+iDigit]>='0' && strText[iChar+1+iDigit]<='9')||
							(strText[iChar+1+iDigit]>='A' && strText[iChar+1+iDigit]<='F')||
							(strText[iChar+1+iDigit]>='a' && strText[iChar+1+iDigit]<='f')))
							bWellFormed=false;
					if(bWellFormed)
					{
						wCurrColor=((HexCharToNumber(strText[iChar+1])&0xf)<<12) |
									((HexCharToNumber(strText[iChar+2])&0xf)<<8) |
									((HexCharToNumber(strText[iChar+3])&0xf)<<4) |
									(HexCharToNumber(strText[iChar+4])&0xf);
						iChar+=5;
					}
				}
				//else simply write the string for editing
			}
		}
		//would we exceed max width when writing this character?
		if(iCurrLinePos+m_aaiFontSpacings[strText[iChar]][1]-m_aaiFontSpacings[strText[iChar]][0]>=iMaxWidth
			&& iCurrLinePos && strText[iChar]!=' ' && strText[iChar]!='\n' && strText[iChar]!='\t')
		{//break the line
			pChars[nChars].cLetter='\n';
			pChars[nChars].iOffset=iCurrLinePos;
			pChars[nChars++].wColor=wCurrColor;
			iCurrLinePos=0;
			iActualHeight++;
		}
		//check if we have a non-alphanumerical character
		if(!std::isalnum(UNSIGN_CHAR(strText[iChar])))
		{
			if(strText[iChar]=='\n')
			{
/*				pChars[nChars].cLetter='\n';
				pChars[nChars].iOffset=iCurrLinePos;
				pChars[nChars++].wColor=wCurrColor;
*/				iCurrLinePos=0;
				iActualHeight++;
			}
			if(strText[iChar]=='\t')
			{
				int iTabEnd=(iCurrLinePos%iTabWidth)+iTabWidth;
				pChars[nChars].cLetter='\t';
				pChars[nChars].iOffset=iCurrLinePos;
				pChars[nChars++].wColor=wCurrColor;
				iCurrLinePos=iTabEnd;
				if(iTabEnd>iMaxWidth)
				{
					pChars[nChars].cLetter='\n';
					pChars[nChars].iOffset=iMaxWidth-1;
					pChars[nChars++].wColor=wCurrColor;
					iCurrLinePos=0;
					iActualHeight++;
				}
			}
			bool bForceLineBreak=false;
			if(strText[iChar]==' ' && iCurrLinePos+m_aaiFontSpacings[' '][1]-m_aaiFontSpacings[' '][0]>=iMaxWidth)
				bForceLineBreak=true;
			
			//write the character
			pChars[nChars].cLetter=UNSIGN_CHAR(strText[iChar]);
			pChars[nChars].iOffset=iCurrLinePos;
			pChars[nChars++].wColor=wCurrColor;
			iCurrLinePos+=m_aaiFontSpacings[pChars[nChars-1].cLetter][1]-m_aaiFontSpacings[pChars[nChars].cLetter][0];
			if(iCurrLinePos>iActualWidth)
				iActualWidth=iCurrLinePos;
			
			//check if there must be a line break for word wrapping
			int iNextWordEnd=iCurrLinePos;
			int iTempChar=iChar+1;
			while(std::isalnum(UNSIGN_CHAR(strText[iTempChar])))
			{
				iNextWordEnd+=m_aaiFontSpacings[UNSIGN_CHAR(strText[iTempChar])][1]-m_aaiFontSpacings[UNSIGN_CHAR(strText[iTempChar])][0];
				iTempChar++;
			}
			if((iNextWordEnd>=iMaxWidth || bForceLineBreak) && iCurrLinePos)
			{//wrap around
				pChars[nChars].cLetter='\n';
				pChars[nChars].iOffset=iCurrLinePos;
				pChars[nChars++].wColor=wCurrColor;
				iCurrLinePos=0;
				iActualHeight++;
			}
		}
		else
		{
			//write the character
			pChars[nChars].cLetter=UNSIGN_CHAR(strText[iChar]);
			pChars[nChars].iOffset=iCurrLinePos;
			pChars[nChars++].wColor=wCurrColor;
			iCurrLinePos+=m_aaiFontSpacings[pChars[nChars-1].cLetter][1]-m_aaiFontSpacings[pChars[nChars-1].cLetter][0];
			if(iCurrLinePos>iActualWidth)
				iActualWidth=iCurrLinePos;
		}
		if(iActualHeight>=iMaxHeight)
		{//luckily this will happen before any characters are placed in the line
			iActualHeight=iMaxHeight-1;
			//----------- EVIL HACK, have to fix this for release!!!! ------------------
			delete pChars;
			return CreateTextTexture(strText.substr(1), iMaxWidth, iMaxHeight, iActualWidth, iActualHeight);
			//--------------------------------------------------------------------------
			break;
		}
	}
	iActualHeight++;
	iActualWidth++;
	Ogre::TexturePtr tex = Ogre::TextureManager::getSingleton().createManual(Ogre::String("SGTFontTexture ") + SGTSceneManager::Instance().RequestIDStr(),
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, iActualWidth, iActualHeight, 0, Ogre::PF_BYTE_BGRA, Ogre::TU_DEFAULT);

	// Get the pixel buffer
	Ogre::HardwarePixelBufferSharedPtr pixelBuffer = tex->getBuffer();

	// Lock the pixel buffer and get a pixel box
	pixelBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD);

	void* pPixels=pixelBuffer->getCurrentLock().data;

	for(int iLine=0; iLine<iActualHeight; iLine++)
		for(int iPixel=0; iPixel<iActualWidth; iPixel++)
			((unsigned int*)(((char*)pPixels)+pixelBuffer->getCurrentLock().rowPitch*iLine))[iPixel]=0;

	int iCurrLine=0;
	for(iChar=0; iChar<(unsigned int)nChars; iChar++)
	{
		unsigned int* pdwCharBegin=((unsigned int*)pPixels+pixelBuffer->getCurrentLock().rowPitch*iCurrLine)+pChars[iChar].iOffset;
		int iLineEnd=iActualWidth-pChars[iChar].iOffset;
		if(pChars[iChar].cLetter=='\n')
		{
			iCurrLine++;
			continue;
		}

		if(pChars[iChar].cLetter=='\t')
		{
			//put some ' ' in there
			int iTabEnd=(pChars[iChar].iOffset%iTabWidth)+iTabWidth;
			for(int iUnit=0; iUnit<iTabEnd-pChars[iChar].iOffset && iUnit<iLineEnd; iUnit++)
			{
				pdwCharBegin[iUnit]=(pChars[iChar].cLetter<<16) |
									((iUnit%(m_aaiFontSpacings[' '][1]-m_aaiFontSpacings[' '][0])+m_aaiFontSpacings[' '][0])<<8) | pChars[iChar].wColor;
			}
			continue;
		}
		//not '\t'
		for(int iUnit=0; iUnit<m_aaiFontSpacings[pChars[iChar].cLetter][1]-m_aaiFontSpacings[pChars[iChar].cLetter][0] && iUnit<iLineEnd; iUnit++)
		{
			//BGRA
			pdwCharBegin[iUnit]=(pChars[iChar].cLetter<<24) |
				((iUnit+m_aaiFontSpacings[pChars[iChar].cLetter][0])<<16) | pChars[iChar].wColor;
		}
	}
	pixelBuffer->unlock();
	return tex;
}

Ogre::MaterialPtr
SGTFontTextures::CreateTextMaterial(Ogre::TexturePtr pTex, Ogre::String strBaseMaterial, Ogre::String strTargetPassName, Ogre::String strTargetTextureAlias, int iMaxWidth, int iMaxHeight)
{
	Ogre::MaterialPtr mat=Ogre::MaterialManager::getSingleton().getByName(strBaseMaterial);
	Ogre::String strTestMatName=Ogre::String("SGTFontTextureMaterial ") + SGTSceneManager::Instance().RequestIDStr();
	mat=mat->clone(strTestMatName);
	//mat now contains the new material
	for(unsigned int iTechnique=0; iTechnique<mat->getNumTechniques(); iTechnique++)
	{
		Ogre::Pass* pPass;
		Ogre::TextureUnitState* pTUS;
		if((pPass=mat->getTechnique(iTechnique)->getPass(strTargetPassName))!=0)
		{
			if((pTUS=pPass->getTextureUnitState(strTargetTextureAlias))!=0)
				pTUS->setTextureName(pTex->getName());
			pPass->getFragmentProgramParameters()->setNamedConstant("fTextWidth", Ogre::Real(pTex->getWidth()));
			pPass->getFragmentProgramParameters()->setNamedConstant("fTextHeight", Ogre::Real(pTex->getHeight()));
			pPass->getFragmentProgramParameters()->setNamedConstant("fTexelSize", Ogre::Real(1.0/(double)m_TexelWidth));
			pPass->getFragmentProgramParameters()->setNamedConstant("fUScale", Ogre::Real((double)iMaxWidth/(double)pTex->getWidth()));
			pPass->getFragmentProgramParameters()->setNamedConstant("fVScale", Ogre::Real((double)iMaxHeight/(double)pTex->getHeight()));
		}
	}
	mat->compile(true);
	return mat;
}

void
SGTFontTextures::ChangeTextMaterial(Ogre::MaterialPtr pMat, Ogre::TexturePtr pTex, Ogre::String strTargetPassName, Ogre::String strTargetTextureAlias, int iMaxWidth, int iMaxHeight)
{
	pMat->unload();
	for(unsigned int iTechnique=0; iTechnique<pMat->getNumTechniques(); iTechnique++)
	{
		Ogre::Pass* pPass;
		Ogre::TextureUnitState* pTUS;
		if((pPass=pMat->getTechnique(iTechnique)->getPass(strTargetPassName))!=0)
		{
			if((pTUS=pPass->getTextureUnitState(strTargetTextureAlias))!=0)
				pTUS->setTextureName(pTex->getName());
			pPass->getFragmentProgramParameters()->setNamedConstant("fTextWidth", Ogre::Real(pTex->getWidth()));
			pPass->getFragmentProgramParameters()->setNamedConstant("fTextHeight", Ogre::Real(pTex->getHeight()));
			pPass->getFragmentProgramParameters()->setNamedConstant("fTexelSize", Ogre::Real(1.0/(double)m_TexelWidth));
			pPass->getFragmentProgramParameters()->setNamedConstant("fUScale", Ogre::Real((double)iMaxWidth/(double)pTex->getWidth()));
			pPass->getFragmentProgramParameters()->setNamedConstant("fVScale", Ogre::Real((double)iMaxHeight/(double)pTex->getHeight()));
		}
	}
	pMat->compile(true);
}