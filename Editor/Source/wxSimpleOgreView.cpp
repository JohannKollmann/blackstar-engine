
#include "wxSimpleOgreView.h"
#include "IceMain.h"


wxSimpleOgreView::wxSimpleOgreView(wxWindow *parent) : wxOgre(parent, -1)
{
	mNode = Ice::Main::Instance().GetPreviewSceneMgr()->getRootSceneNode()->createChildSceneNode(Ogre::Vector3(0, 1000, 0));
}

wxSimpleOgreView::~wxSimpleOgreView(void)
{
}

void wxSimpleOgreView::ShowMesh(Ogre::String meshFileName)
{
	Reset();
	Ogre::Entity *ent;
}

void wxSimpleOgreView::ShowParticleEffect(Ogre::String pfxName)
{
	Reset();
}

void wxSimpleOgreView::ShowTexture(Ogre::String texFileName)
{
	Reset();
}

void wxSimpleOgreView::Reset()
{
	for (unsigned int i = 0; i < mNode->numAttachedObjects(); i++)
	{
		mNode->getCreator()->destroyMovableObject(mNode->getAttachedObject(i));
	}
}