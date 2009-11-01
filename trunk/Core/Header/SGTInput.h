
#ifndef __SGTInput_H__
#define __SGTInput_H__

#ifdef _DEBUG
#pragma comment(lib, "OIS_d.lib")
#else
#pragma comment(lib, "OIS.lib")
#endif
#define OIS_DYNAMIC_LIB
#include "OIS/OIS.h"

#include "SGTIncludes.h"
#include "Ogre.h"
#include "SGTMessageListener.h"

enum SGTDllExport EMouseButtons
{
	MOUSE_LEFT,
	MOUSE_RIGHT,
	MOUSE_BOTH,
	MOUSE_NONE
};

class SGTDllExport SGTInput : public OIS::KeyListener, public OIS::MouseListener
{

private:
	OIS::InputManager *mInputmanager;
	OIS::Mouse*    mMouse;
	OIS::Keyboard* mKeyboard;

public:
	SGTInput(size_t windowHnd, int width, int height, bool freeCursor = false);
	~SGTInput(void);

	bool isKeyDown(OIS::KeyCode kc);

	OIS::MouseState getMouseState();
	EMouseButtons getMouseButton();

	Ogre::String GetKeyCodeAsString(OIS::KeyCode kc);

	//Callbackfunktionen
	bool keyPressed( const OIS::KeyEvent &arg );
	bool keyReleased( const OIS::KeyEvent &arg );
	bool mouseMoved(const OIS::MouseEvent &e);
	bool mousePressed (const OIS::MouseEvent &,OIS::MouseButtonID id);
	bool mouseReleased (const OIS::MouseEvent &,OIS::MouseButtonID id);

	void Update();
};

#endif
