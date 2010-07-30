#include "Ogre.h"

class __declspec(dllexport) FontTextures
{
public:
	FontTextures(std::string strFontSpacings);
	Ogre::TexturePtr CreateTextTexture(std::string strText, int iMaxWidth, int iMaxHeight, int& iActualWidth, int& iActualHeight);
	Ogre::MaterialPtr CreateTextMaterial(Ogre::TexturePtr pTex, Ogre::String strBaseMaterial, Ogre::String strTargetPassName, Ogre::String strTargetTextureAlias, int iMaxWidth, int iMaxHeight);
	void ChangeTextMaterial(Ogre::MaterialPtr pMat, Ogre::TexturePtr pTex, Ogre::String strTargetPassName, Ogre::String strTargetTextureAlias, int iMaxWidth, int iMaxHeight);
private:
	int m_aaiFontSpacings[256][2];
	int m_TexelWidth;
	struct SCharInfo
	{
		unsigned char cLetter;
		unsigned short wColor;
		int iOffset;
	};

	static int
	HexCharToNumber(char c)
	{
		int r=-1;
		if(c>='a' && c<='f')
			r=10+c-'a';
		if(c>='A' && c<='F')
			r=10+c-'A';
		if(c>='0' && c<='9')
			r=c-'0';
		return r;
	}
};