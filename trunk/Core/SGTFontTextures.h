#include "Ogre.h"
#include "SGTSceneManager.h"

class SGTFontTextures
{
public:
	SGTFontTextures(std::string strFontSpacings);
	Ogre::TexturePtr CreateTextTexture(std::string strText, int iMaxWidth, int iMaxHeight, int& iActualWidth, int& iActualHeight);
	Ogre::MaterialPtr CreateTextMaterial(Ogre::TexturePtr pTex, Ogre::String strBaseMaterial, Ogre::String strTargetPassName, Ogre::String strTargetTextureAlias, int iMaxWidth, int iMaxHeight);
private:
	int m_aaiFontSpacings[256][2];
	int m_TexelWidth;
	struct SCharInfo
	{
		char cLetter;
		unsigned short wColor;
		int iOffset;
	};

	static int
	HexCharToNumber(char c)
	{
		int r=-1;
		if(c>='a' && c<='f')
			r=10+c-'a';
		if(c>='0' && c<='9')
			r=c-'0';
		return r;
	}
};