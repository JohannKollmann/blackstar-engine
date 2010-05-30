
#pragma once

#include "wxOgre.h"

class wxSimpleOgreView : public wxOgre
{
private:
	Ogre::SceneNode *mNode;
public:
	wxSimpleOgreView(wxWindow* parent);
	~wxSimpleOgreView();

	void ShowMesh(Ogre::String meshFileName);
	void ShowParticleEffect(Ogre::String pfxName);
	void ShowTexture(Ogre::String texFileName);
	void Reset();
};

