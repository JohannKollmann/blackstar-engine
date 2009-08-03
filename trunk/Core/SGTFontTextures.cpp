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
				if(strText.size()-iChar<6)
					break;
				wCurrColor=((HexCharToNumber(strText[iChar+1])&0xf)<<12) |
							((HexCharToNumber(strText[iChar+2])&0xf)<<8) |
							((HexCharToNumber(strText[iChar+3])&0xf)<<4) |
							(HexCharToNumber(strText[iChar+4])&0xf);
				iChar+=5;
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
		if(!std::isalnum(strText[iChar]))
		{
			if(strText[iChar]=='\n')
			{
				pChars[nChars].cLetter='\n';
				pChars[nChars].iOffset=iCurrLinePos;
				pChars[nChars++].wColor=wCurrColor;
				iCurrLinePos=0;
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
			pChars[nChars].cLetter=strText[iChar];
			pChars[nChars].iOffset=iCurrLinePos;
			pChars[nChars++].wColor=wCurrColor;
			iCurrLinePos+=m_aaiFontSpacings[strText[iChar]][1]-m_aaiFontSpacings[strText[iChar]][0];
			if(iCurrLinePos>iActualWidth)
				iActualWidth=iCurrLinePos;
			
			//check if there must be a line break for word wrapping
			int iNextWordEnd=iCurrLinePos;
			int iTempChar=iChar+1;
			while(std::isalnum(strText[iTempChar]))
			{
				iNextWordEnd+=m_aaiFontSpacings[strText[iTempChar]][1]-m_aaiFontSpacings[strText[iTempChar]][0];
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
			pChars[nChars].cLetter=strText[iChar];
			pChars[nChars].iOffset=iCurrLinePos;
			pChars[nChars++].wColor=wCurrColor;
			iCurrLinePos+=m_aaiFontSpacings[strText[iChar]][1]-m_aaiFontSpacings[strText[iChar]][0];
			if(iCurrLinePos>iActualWidth)
				iActualWidth=iCurrLinePos;
		}
		if(iActualHeight>=iMaxHeight)
		{//luckily this will happen before any characters are placed in the line
			iActualHeight=iMaxHeight-1;
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
			{
				pTUS->setTextureName(pTex->getName());
			}
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

/*

	int nWidth=iTickWidth;
	SCharInfo* pChars=new SCharInfo[strText.size()];
	int nLines=0;
	int iCurrLinePos=0;
	unsigned short wCurrColor=0xf;
	unsigned int iChar=0;
	for(; iChar<strText.size(); iChar++)
	{
		//treat some special characters..
		switch(strText[iChar])
		{
		case '$':
			if(strText[iChar+1]=='$')//the escape character itself
			{
				//make that escape char disappear
				pChars[iChar].cLetter=' ';
				pChars[iChar].iLine=nLines;
				pChars[iChar].wColor=wCurrColor;
				pChars[iChar].iWidth=0;//actually only this is important
				pChars[iChar].iOffset=iCurrLinePos;
				iChar++;
			}
			else
			{
				wCurrColor=(HexCharToNumber(strText[iChar+1])<<12) |
							(HexCharToNumber(strText[iChar+2])<<8) |
							(HexCharToNumber(strText[iChar+3])<<4) |
							HexCharToNumber(strText[iChar+4]);
				//make these 5 letters disappear
				for(int iLetter=0; iLetter<5; iLetter++)
				{
					pChars[iChar+iLetter].cLetter=' ';
					pChars[iChar+iLetter].iLine=nLines;
					pChars[iChar+iLetter].wColor=wCurrColor;
					pChars[iChar+iLetter].iWidth=0;//actually only this is important
					pChars[iChar+iLetter].iOffset=iCurrLinePos;
				}
				iChar+=4;
				continue;
			}
			break;
		case '\t':
		{
			int iTabWidth=4*(m_aaiFontSpacings[' '][1]-m_aaiFontSpacings[' '][0]);
			pChars[iChar].iOffset=iCurrLinePos;
			iCurrLinePos/=iTabWidth;
			iCurrLinePos=(iCurrLinePos+1)*iTabWidth;
			pChars[iChar].cLetter='\t';
			pChars[iChar].iLine=nLines;
			pChars[iChar].wColor=wCurrColor;
			pChars[iChar].iWidth=iCurrLinePos-pChars[iChar].iOffset;
		}
		case ' '://fall through from '\t'
		{
			//make nice word wraps ;)
			if(!iCurrLinePos)
				break;//we already are in a new line, no good idea to think about wrapping here

			int iWordLength=(strText[iChar]==' ') ? m_aaiFontSpacings[' '][1]-m_aaiFontSpacings[' '][0] : 0;
			for(unsigned int iSearchPos=iChar+1; iSearchPos<strText.size() &&
				strText[iSearchPos]!=' ' && strText[iSearchPos]!='\n' && strText[iSearchPos]!='\t'; iSearchPos++)
				iWordLength+=m_aaiFontSpacings[strText[iSearchPos]][1]-m_aaiFontSpacings[strText[iSearchPos]][0];
			//now we got the size of the next word, test if it fits in the current line
			if(iCurrLinePos+iWordLength>=nWidth)
			//doesn't fit.. wrap around. do this by replacing the ' ' or '\t' by '\n'
				strText[iChar]='\n';
			else
			{
				if(strText[iChar]==' ')
					break;
				else
					continue;
			}
		}
			//fall through if we replaced the letter
		case '\n':
			iCurrLinePos=0;
			pChars[iChar].iOffset=0;
			pChars[iChar].iWidth=0;
			pChars[iChar].iLine=0;
			nLines++;
			continue;
		}
		if(iChar>=strText.size())//we jumped over the ending of the text..
			break;
		int iLetterWidth=m_aaiFontSpacings[strText[iChar]][1]-
			m_aaiFontSpacings[strText[iChar]][0];

		//break the line if the letter does not fit
		if((iCurrLinePos+iLetterWidth)>nWidth)
		{
			nLines++;
			iCurrLinePos=0;
		}
		pChars[iChar].cLetter=strText[iChar];
		pChars[iChar].iLine=nLines;
		pChars[iChar].wColor=wCurrColor;
		pChars[iChar].iWidth=iLetterWidth;
		pChars[iChar].iOffset=iCurrLinePos;

		iCurrLinePos+=iLetterWidth;
	}
	if(iCurrLinePos)
		nLines++;//make it the number of lines instead of an index

	
	Ogre::TexturePtr tex = Ogre::TextureManager::getSingleton().createManual(Ogre::String("SGTFontTexture ") + SGTSceneManager::Instance().RequestIDStr(),
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, nWidth, nLines, 0, Ogre::PF_BYTE_BGRA, Ogre::TU_DEFAULT);

	// Get the pixel buffer
	Ogre::HardwarePixelBufferSharedPtr pixelBuffer = tex->getBuffer();

	// Lock the pixel buffer and get a pixel box
	pixelBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD);

	void* pPixels=pixelBuffer->getCurrentLock().data;

	for(int iLine=0; iLine<nLines; iLine++)
		for(int iPixel=0; iPixel<nWidth; iPixel++)
			((unsigned int*)(((char*)pPixels)+pixelBuffer->getCurrentLock().rowPitch*iLine))[iPixel]=0;

	for(iChar=0; iChar<strText.size(); iChar++)
	{
		unsigned int* pdwCharBegin=((unsigned int*)(((char*)pPixels)+pixelBuffer->getCurrentLock().rowPitch*pChars[iChar].iLine))+pChars[iChar].iOffset;
		if(pChars[iChar].cLetter=='\t')
		{
			//put some ' ' in there
			for(int iUnit=0; iUnit<pChars[iChar].iWidth; iUnit++)
			{
				int iSpaceWidth=m_aaiFontSpacings[' '][1]-m_aaiFontSpacings[' '][0];
				pdwCharBegin[iUnit]=' ' |
											(((iUnit+iSpaceWidth-pChars[iChar].iOffset)%iSpaceWidth+m_aaiFontSpacings[' '][0])<<8) |
											((pChars[iChar].wColor&0xff)<<24) |
											((pChars[iChar].wColor&0xff00)<<8);
			}
			continue;
		}
		//not '\t'
		for(int iUnit=0; iUnit<pChars[iChar].iWidth; iUnit++)
		{
			/*pdwCharBegin[iUnit]=pChars[iChar].cLetter |
										((iUnit+m_aaiFontSpacings[pChars[iChar].cLetter][0])<<8) |
										((pChars[iChar].wColor&0xff)<<24) |
										((pChars[iChar].wColor&0xff00)<<8);*//*
			//BGRA
			pdwCharBegin[iUnit]=(pChars[iChar].cLetter<<16) |
										((iUnit+m_aaiFontSpacings[pChars[iChar].cLetter][0])<<8) | 0x000000ff;
		}
	}

		// Unlock the pixel buffer
	pixelBuffer->unlock();
	return tex;
	*/