
#include "SGTSceneListener.h"

SGTSceneListener::SGTSceneListener(void)
{
}

SGTSceneListener::~SGTSceneListener(void)
{
}


void SGTSceneListener::shadowTextureCasterPreViewProj(Ogre::Light *light, Ogre::Camera *cam)
{
	float range = light->getAttenuationRange();
	cam->setNearClipDistance(range * 0.01);
	cam->setFarClipDistance(range);
}