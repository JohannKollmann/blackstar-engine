
#pragma once

#include "wxOgre.h"

class wxSimpleOgreView : public wxOgre
{
private:
	Ogre::SceneNode *mNode;
	Ogre::SceneManager *mSceneMgr;
	Ogre::Viewport *mViewport;

protected:
	void OnRender();
	void OnInit();

public:
	wxSimpleOgreView(wxWindow* parent, wxWindowID id,
           const wxPoint& pos = wxDefaultPosition,
           const wxSize& size = wxDefaultSize, long style = 0);
	~wxSimpleOgreView();

	void ShowMesh(Ogre::String meshFileName);
	void ShowParticleEffect(Ogre::String pfxName);
	void ShowTexture(Ogre::String texFileName);
	void Reset();

	void Update(float time);
};

