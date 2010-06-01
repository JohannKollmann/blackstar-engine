
#include "wxTextureDisplay.h"
#include "wxMainMenu.h"
#include "wxEdit.h"
#include "OgreOggSound.h"

IMPLEMENT_CLASS(wxTextureDisplay, wxControl)

BEGIN_EVENT_TABLE(wxTextureDisplay, wxControl)
    EVT_PAINT(wxTextureDisplay::OnPaint)
	EVT_ERASE_BACKGROUND(wxTextureDisplay::OnEraseBackground)
	EVT_SHOW(wxTextureDisplay::OnShow)
END_EVENT_TABLE()

wxTextureDisplay::wxTextureDisplay(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
: wxControl(parent, id, pos, size)
{
	mBitmap = wxBitmap();
	mTexture.setNull();
	mPreviewNode = 0;
	mTempDataSize = 512 * 512 * 4;
	mTempData = new unsigned char[mTempDataSize];
}

wxTextureDisplay::~wxTextureDisplay()
{
	if (mTempData) delete mTempData;
}

void wxTextureDisplay::OnShow(wxShowEvent& event)
{
	wxEdit::Instance().GetMainMenu()->Check(wxMainMenu_PreviewWindow, event.GetShow());
}

void wxTextureDisplay::BakeBitmap()
{
	if (!mTexture.isNull())
	{
		unsigned int bytesize = mTexture->getBuffer()->getSizeInBytes();
		if (bytesize > mTempDataSize)
		{
			mTempDataSize = bytesize;
			delete mTempData;
			mTempData = new unsigned char[mTempDataSize]; 
		}
		Ogre::PixelBox pixelBox(mTexture->getWidth(), mTexture->getHeight(), mTexture->getDepth(), mTexture->getFormat(), mTempData);
		mTexture->getBuffer()->blitToMemory(pixelBox);

		wxImage image(pixelBox.getWidth(), pixelBox.getHeight());

		size_t pitch = pixelBox.rowPitch;
		size_t width = 0;
		size_t height = 0;
		if (pixelBox.format == Ogre::PixelFormat::PF_A8R8G8B8) image.InitAlpha();
		unsigned int format_size = Ogre::PixelUtil::getNumElemBytes(pixelBox.format);

		unsigned char* pcRGBs=(unsigned char*)malloc(mTexture->getWidth()*mTexture->getHeight()*3);
		unsigned char* pcAlphas;
		if(image.HasAlpha())
			pcAlphas=(unsigned char*)malloc(pixelBox.getWidth()*pixelBox.getHeight());

		for (unsigned int i = 0; i < bytesize-format_size; i+=format_size)
		{
			Ogre::ColourValue color;
			Ogre::PixelUtil::unpackColour(&color, pixelBox.format, mTempData+i);
			
			int offset=3*(width+pixelBox.getWidth()*height);
			Ogre::ARGB rgbVal=color.getAsARGB();
			pcRGBs[offset]=(rgbVal&0x00ff0000)>>16;
			pcRGBs[offset+1]=(rgbVal&0x0000ff00)>>8;
			pcRGBs[offset+2]=(rgbVal&0xff);

			if(image.HasAlpha())
				pcAlphas[offset/3]=(rgbVal&0xff000000)>>24;

			width++;
			if (width == pitch)
			{
				width = 0;
				height++;
			}
		}
		image.SetData(pcRGBs);
		if (image.HasAlpha())
			image.SetAlpha(pcAlphas);
	
		mBitmap = wxBitmap(image.Scale(GetSize().GetWidth(), GetSize().GetHeight(), wxIMAGE_QUALITY_NORMAL));
	}
}

void wxTextureDisplay::OnEraseBackground( wxEraseEvent& )
{
	BakeBitmap();
}

void wxTextureDisplay::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc( this );
	if (mBitmap.IsOk()) dc.DrawBitmap(mBitmap, 0, 0, true);
}

void wxTextureDisplay::SetTexture(Ogre::TexturePtr texture)
{
	mTexture = texture;
	Refresh();
}

void wxTextureDisplay::Update(float time)
{
	if (mPreviewNode)
	{
		mPreviewNode->yaw(Ogre::Radian(time * 0.5f));
		Refresh();
	}
}

void wxTextureDisplay::SetPreviewNode(Ogre::SceneNode *node)
{
	mPreviewNode = node;
}
void wxTextureDisplay::ClearDisplay()
{
	if (mPreviewNode)
	{
		for (unsigned int i = 0; i < mPreviewNode->numAttachedObjects(); i++)
		{
			OgreOggSound::OgreOggISound *sound = dynamic_cast<OgreOggSound::OgreOggISound*>(mPreviewNode->getAttachedObject(i));
			if (sound) sound->stop();
			mPreviewNode->getCreator()->destroyMovableObject(mPreviewNode->getAttachedObject(i));
		}
		mPreviewNode = nullptr;
	}
	mTexture.setNull();
	mBitmap = wxBitmap();
	ClearBackground();
	Refresh();
}

void wxTextureDisplay::ClearTexture()
{
	mTexture.setNull();
}