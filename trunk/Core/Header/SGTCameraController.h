
#ifndef __SGTCameraController_H__
#define __SGTCameraController_H__

#include "SGTIncludes.h"
#include "Ogre.h"
#include "SGTMessageListener.h"

class SGTDllExport SGTCameraController : public SGTMessageListener
{
private:
	Ogre::Camera *mCamera;
	void Reset();

public:
	SGTCameraController(void);
	~SGTCameraController(void);

	Ogre::Vector3 mMovementVec;
	bool mYRot;
	bool mXRot;
	bool mMove;
	float mRotSpeed;
	float mMoveSpeed;

	Ogre::Camera* GetCamera() { return mCamera; };
	void AttachToNode(Ogre::SceneNode *node, Ogre::Vector3 offset);


	void ReceiveMessage(SGTMsg &msg);

};

#endif