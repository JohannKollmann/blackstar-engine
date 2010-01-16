
#include "IceSceneListener.h"

namespace Ice
{

SceneListener::SceneListener(void)
{
}

SceneListener::~SceneListener(void)
{
}


void SceneListener::shadowTextureCasterPreViewProj(Ogre::Light *light, Ogre::Camera *cam)
{
	float range = light->getAttenuationRange();
	cam->setNearClipDistance(range * 0.01);
	cam->setFarClipDistance(range);
}

};