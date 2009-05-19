
#include "SGTInput.h"
#include "SGTMessageSystem.h"
#include "SGTMain.h"
#include "SGTMsg.h"


SGTInput::SGTInput(size_t windowHnd, int width, int height, bool freeCursor)
{
	Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
	OIS::ParamList pl;	

	std::ostringstream windowHndStr;
	windowHndStr << windowHnd;
	Ogre::LogManager::getSingletonPtr()->logMessage(windowHndStr.str().c_str());

	pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

	if (freeCursor == true)
	{
		pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));
		pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND")));
		pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_NONEXCLUSIVE")));
		pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_FOREGROUND")));
	}

	Ogre::LogManager::getSingletonPtr()->logMessage("creating Input System");
	mInputmanager = OIS::InputManager::createInputSystem( pl );
	Ogre::LogManager::getSingletonPtr()->logMessage("Input System created...");
	try
	{
		mKeyboard = static_cast<OIS::Keyboard*>(mInputmanager->createInputObject( OIS::OISKeyboard, false ));
	}
	catch( const OIS::Exception &ex )
	{
		Ogre::LogManager::getSingletonPtr()->logMessage(ex.eText);
	}

	mMouse = static_cast<OIS::Mouse*>(mInputmanager->createInputObject( OIS::OISMouse, true ));
	mMouse->setEventCallback(this);
	mMouse->setBuffered(true);
	const OIS::MouseState &mouseState = mMouse->getMouseState();
	mouseState.width  = width;
	mouseState.height = height;
	mKeyboard->setEventCallback(this);
	mKeyboard->setBuffered(true);

	SGTMessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");

	Ogre::LogManager::getSingleton().logMessage("SGTInput wurde erfolgreich initialisiert!");
};

SGTInput::~SGTInput(void)
{
	OIS::InputManager::destroyInputSystem(mInputmanager);
};

bool SGTInput::isKeyDown(OIS::KeyCode kc)
{
	return mKeyboard->isKeyDown(kc);
};

Ogre::String SGTInput::GetKeyCodeAsString(OIS::KeyCode kc)
{
	return Ogre::String(mKeyboard->getAsString(kc).c_str());
}

OIS::MouseState SGTInput::getMouseState()
{
	return mMouse->getMouseState();
};

EMouseButtons SGTInput::getMouseButton()
{
	const OIS::MouseState &ms = mMouse->getMouseState();
	if (ms.buttons == 1) return MOUSE_LEFT;
	else if (ms.buttons == 2) return MOUSE_RIGHT;
	else if (ms.buttons == 3) return MOUSE_BOTH;
	else return MOUSE_NONE;
};

bool SGTInput::keyPressed( const OIS::KeyEvent &arg )
{
	SGTMsg msg;
	msg.mNewsgroup = "KEY_DOWN";
	msg.mData.AddInt("KEY_ID_OIS", arg.key);
	msg.mData.AddInt("KEY_ID", arg.text);
	SGTMessageSystem::Instance().SendMessage(msg);

	return true;
};

bool SGTInput::keyReleased( const OIS::KeyEvent &arg )
{
	SGTMsg msg;
	msg.mNewsgroup = "KEY_UP";
	msg.mData.AddInt("KEY_ID_OIS", arg.key);
	msg.mData.AddInt("KEY_ID", arg.text);
	SGTMessageSystem::Instance().SendMessage(msg);
	return true;
};

bool SGTInput::mouseMoved(const OIS::MouseEvent &e)
{
	SGTMsg msg;
	msg.mNewsgroup = "MOUSE_MOVE";
	msg.mData.AddInt("ROT_X_ABS", e.state.X.abs);
	msg.mData.AddInt("ROT_X_REL", e.state.X.rel);
	msg.mData.AddInt("ROT_Y_ABS", e.state.Y.abs);
	msg.mData.AddInt("ROT_Y_REL", e.state.Y.rel);
	SGTMessageSystem::Instance().SendMessage(msg);
	return true;
};

bool SGTInput::mousePressed (const OIS::MouseEvent &,OIS::MouseButtonID id)
{
	SGTMsg msg;
	msg.mNewsgroup = "MOUSE_DOWN";
	msg.mData.AddInt("MOUSE_ID", id);
	SGTMessageSystem::Instance().SendMessage(msg);
	return true;
};

bool SGTInput::mouseReleased (const OIS::MouseEvent &,OIS::MouseButtonID id)
{
	SGTMsg msg;
	msg.mNewsgroup = "MOUSE_UP";
	msg.mData.AddInt("MOUSE_ID", id);
	SGTMessageSystem::Instance().SendMessage(msg);
	return true;
};

void SGTInput::ReceiveMessage(SGTMsg &msg)
{
	if (msg.mNewsgroup == "UPDATE_PER_FRAME")
	{
		mMouse->capture();
		mKeyboard->capture();
		/*float time = msg.mData.GetFloat("TIME");
		Ogre::Vector3 vec = msg.mData.GetOgreVec3("TESTVEC");
		Ogre::Quaternion quat = msg.mData.GetOgreQuat("TESTQUAT");
		Ogre::LogManager::getSingleton().logMessage(Ogre::StringConverter::toString(time) + " " + Ogre::StringConverter::toString(vec) + " " + Ogre::StringConverter::toString(quat));*/
	}
};