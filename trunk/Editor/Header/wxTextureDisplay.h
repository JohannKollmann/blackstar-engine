
#pragma once

#include "wx/aui/aui.h"
#include "wx/wx.h"
#include "Ogre.h"

class wxTextureDisplay : public wxControl
{
	DECLARE_CLASS(wxTextureDisplay) 

private:
	Ogre::TexturePtr mTexture;
	wxBitmap mBitmap;
	Ogre::SceneNode *mPreviewNode;
	unsigned int mTempDataSize;
	unsigned char *mTempData;

protected:
	DECLARE_EVENT_TABLE()
	void OnEraseBackground(wxEraseEvent&);
	void OnPaint(wxPaintEvent& event);
	void OnShow(wxShowEvent& event);


public:
	wxTextureDisplay(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size);
	~wxTextureDisplay();

	void SetTexture(Ogre::TexturePtr texture);
	void BakeBitmap();
	void ClearTexture();
	void Update(float time);
	void SetPreviewNode(Ogre::SceneNode *node);
	void ClearDisplay();
	wxBitmap& GetCurrentBitmap() { return mBitmap; }
};
