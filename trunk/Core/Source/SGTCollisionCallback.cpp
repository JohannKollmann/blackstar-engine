
#include "SGTCollisionCallback.h"
#include "SGTMain.h"

SGTCollisionCallback::SGTCollisionCallback(void)
{
}

SGTCollisionCallback::~SGTCollisionCallback(void)
{
}


void SGTCollisionCallback::onStartTouch(NxOgre::Actor* a, NxOgre::Actor* b, NxOgre::ContactStream* cs)
{
	//Ogre::LogManager::getSingleton().logMessage("Contact: " + Ogre::String(a->getName().c_str()));
}

void SGTCollisionCallback::onEndTouch(NxOgre::Actor* a, NxOgre::Actor* b, NxOgre::ContactStream* cs)
{
}

void SGTCollisionCallback::onTouch(NxOgre::Actor* a, NxOgre::Actor* b, NxOgre::ContactStream* cs)
{
}
