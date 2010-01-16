
#pragma once

#include "IceIncludes.h"
#include "Ogre.h"
#include "IceMessageListener.h"

namespace Ice
{

class DllExport CameraController : public MessageListener
{
private:
	Ogre::Camera *mCamera;
	void Reset();

public:
	CameraController(void);
	~CameraController(void);

	Ogre::Vector3 mMovementVec;
	bool mYRot;
	bool mXRot;
	bool mMove;
	float mRotSpeed;
	float mMoveSpeed;

	Ogre::Camera* GetCamera() { return mCamera; };
	void AttachToNode(Ogre::SceneNode *node, Ogre::Vector3 offset);


	void ReceiveMessage(Msg &msg);

};

};