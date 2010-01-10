
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

	mEnabled = true;

	SGTMessageSystem::Instance().CreateNewsgroup("CONTROL_DOWN");
	SGTMessageSystem::Instance().CreateNewsgroup("CONTROL_UP");

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

	if(m_mKeyControls.find(arg.key)!=m_mKeyControls.end())
	{
		for(unsigned int iControl=0; iControl<m_mKeyControls[arg.key].size(); iControl++)
		{
			SGTMsg msgControl;
			msgControl.mNewsgroup= "CONTROL_DOWN";
			msgControl.mData.AddOgreString("CONTROL_NAME", m_mKeyControls[arg.key][iControl]);
			SGTMessageSystem::Instance().SendMessage(msgControl);
		}
	}

	return true;
};

bool SGTInput::keyReleased( const OIS::KeyEvent &arg )
{
	SGTMsg msg;
	msg.mNewsgroup = "KEY_UP";
	msg.mData.AddInt("KEY_ID_OIS", arg.key);
	msg.mData.AddInt("KEY_ID", arg.text);
	SGTMessageSystem::Instance().SendMessage(msg);

	if(m_mKeyControls.find(arg.key)!=m_mKeyControls.end())
	{
		for(unsigned int iControl=0; iControl<m_mKeyControls[arg.key].size(); iControl++)
		{
			SGTMsg msgControl;
			msgControl.mNewsgroup= "CONTROL_UP";
			msgControl.mData.AddOgreString("CONTROL_NAME", m_mKeyControls[arg.key][iControl]);
			SGTMessageSystem::Instance().SendMessage(msgControl);
		}
	}
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
	
	if(m_mMouseControls.find(id)!=m_mMouseControls.end())
	{
		for(unsigned int iControl=0; iControl<m_mMouseControls[id].size(); iControl++)
		{
			SGTMsg msgControl;
			msgControl.mNewsgroup= "CONTROL_DOWN";
			msgControl.mData.AddOgreString("CONTROL_NAME", m_mMouseControls[id][iControl]);
			SGTMessageSystem::Instance().SendMessage(msgControl);
		}
	}
	return true;
};

bool SGTInput::mouseReleased (const OIS::MouseEvent &,OIS::MouseButtonID id)
{
	SGTMsg msg;
	msg.mNewsgroup = "MOUSE_UP";
	msg.mData.AddInt("MOUSE_ID", id);
	SGTMessageSystem::Instance().SendMessage(msg);
	if(m_mMouseControls.find(id)!=m_mMouseControls.end())
	{
		for(unsigned int iControl=0; iControl<m_mMouseControls[id].size(); iControl++)
		{
			SGTMsg msgControl;
			msgControl.mNewsgroup= "CONTROL_UP";
			msgControl.mData.AddOgreString("CONTROL_NAME", m_mMouseControls[id][iControl]);
			SGTMessageSystem::Instance().SendMessage(msgControl);
		}
	}
	return true;
};

void SGTInput::SetEnabled(bool enable)
{
	mEnabled = enable;
}

void SGTInput::Update()
{
	if (mEnabled)
	{
		mMouse->capture();
		mKeyboard->capture();
	}
};

//key mapped functions

void
SGTInput::SetControl(Ogre::String strName, std::vector<std::pair<OIS::KeyCode, OIS::MouseButtonID>> buttons)
{
	if(m_mControls.find(strName)!=m_mControls.end())
	{
		//delete old bindings
		std::vector<std::pair<OIS::KeyCode, OIS::MouseButtonID>> oldcontrols=m_mControls[strName];
		for(unsigned int iButton=0; iButton<oldcontrols.size(); iButton++)
		{
			if(oldcontrols[iButton].first!=0)
			{
				for(std::vector<Ogre::String>::iterator it=m_mKeyControls[oldcontrols[iButton].first].begin();
					it!=m_mKeyControls[oldcontrols[iButton].first].end(); it++)
					if(*it==strName)
					{
						m_mKeyControls[oldcontrols[iButton].first].erase(it);
						it=m_mKeyControls[oldcontrols[iButton].first].begin();
						if(it==m_mKeyControls[oldcontrols[iButton].first].end())
							break;
					}
			}
			else
				for(std::vector<Ogre::String>::iterator it=m_mMouseControls[oldcontrols[iButton].second].begin();
					it!=m_mMouseControls[oldcontrols[iButton].second].end(); it++)
					if(*it==strName)
					{
						m_mMouseControls[oldcontrols[iButton].second].erase(it);
						it=m_mMouseControls[oldcontrols[iButton].second].begin();
						if(it==m_mMouseControls[oldcontrols[iButton].second].end())
							break;
					}
		}
	}
	m_mControls[strName]=buttons;
	for(unsigned int iButton=0; iButton<buttons.size(); iButton++)
	{
		if(buttons[iButton].first!=0)
			m_mKeyControls[buttons[iButton].first].push_back(strName);
		else
			m_mMouseControls[buttons[iButton].second].push_back(strName);
	}
}

std::vector<std::pair<OIS::KeyCode, OIS::MouseButtonID>>
SGTInput::GetControl(Ogre::String strName)
{
	if(m_mControls.find(strName)==m_mControls.end())
		return std::vector<std::pair<OIS::KeyCode, OIS::MouseButtonID>>();
	else
		return m_mControls[strName];
}

bool
SGTInput::ControlPressed(Ogre::String strName)
{
	if(m_mControls.find(strName)==m_mControls.end())
		return false;
	std::vector<std::pair<OIS::KeyCode, OIS::MouseButtonID>> controls=m_mControls[strName];
	const OIS::MouseState &ms = mMouse->getMouseState();
	for(unsigned int iControl=0; iControl<controls.size(); iControl++)
	{
		if(controls[iControl].first!=0)
		{
			if(isKeyDown(controls[iControl].first))
				return true;
		}
		else
			if(ms.buttonDown(controls[iControl].second))
				return true;
	}
	return false;
}