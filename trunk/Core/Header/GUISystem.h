#ifndef __SGTGUISystem_H__
#define __SGTGUISystem_H__

#include "Ogre.h"
#include "SGTMessageListener.h"
#include "SGTMessageSystem.h"
#include "SGTMain.h"
#include "SGTSceneManager.h"
#include "SGTFontTextures.h"
#include <map>
#include <vector>
#include <list>



class SGTDllExport SGTGUISystem : public SGTMessageListener
{
public:
	SGTGUISystem(void);
	~SGTGUISystem(void);
	static SGTGUISystem& GetInstance(); 

	void ReceiveMessage(SGTMsg &msg);

	class SubWindow;
	class Window;
	friend class SGTGUISystem::SubWindow;
	friend class SGTGUISystem::Window;

	class SGTDllExport Window
	{
	public:
		Window(int iHandle);
		Window();
		void Bake();
		void SetMaterial(Ogre::String mat);
		void Move(float x, float y);
		int GetHandle();
		

	private:
		std::vector<int> GetSubWindows();
		static std::list<int> FindSubWindows(int iHandle);
		int m_iHandle;
	};
	class SGTDllExport SubWindow
	{
	public:
		SubWindow(int iHandle);
		void SetMaterial(Ogre::String mat);
		int GetHandle();

	private:
		int m_iHandle;
	};
	SGTGUISystem::Window MakeWindow(float x, float y, float w, float h, float fUScale=1.0f, float fVScale=1.0f);
	SGTGUISystem::SubWindow CreateSubWindow(int iParentHandle, float x, float y, float w, float h, float fUScale=1.0f, float fVScale=1.0f);
	void SetFocus(int iHandle);
	void SetForegroundWindow(int iHandle);
	void SetVisible(int iHandle, bool bVisible);
	bool GetVisible(int iHandle);
	void SetCursor(int iHandle);

		
private:
	//scripting stuff

	static std::vector<SGTScriptParam> Lua_CreateWindow(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_CreateSubWindow(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_BakeWindow(SGTScript& caller, std::vector<SGTScriptParam> vParams);

	static std::vector<SGTScriptParam> Lua_SetMaterial(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_SetWindowVisible(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_SetForegroundWindow(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_SetFocus(SGTScript& caller, std::vector<SGTScriptParam> vParams);

	static std::vector<SGTScriptParam> Lua_GetScreenCoords(SGTScript& caller, std::vector<SGTScriptParam> vParams);

	static std::vector<SGTScriptParam> Lua_GetCursor(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_SetCursor(SGTScript& caller, std::vector<SGTScriptParam> vParams);

	//font system
	static std::vector<SGTScriptParam> Lua_CreateFontMaterial(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_CreateFontTexture(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_ChangeFontMaterial(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_DeleteTexture(SGTScript& caller, std::vector<SGTScriptParam> vParams);

	//window callbacks
	//mouse events
	static std::vector<SGTScriptParam> Lua_SetOnClickCallback(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_SetMouseDownCallback(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_SetMouseUpCallback(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_SetMouseMoveCallback(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_SetMouseHoverInCallback(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_SetMouseHoverOutCallback(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	//keyboard events
	static std::vector<SGTScriptParam> Lua_SetOnCharCallback(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_SetKeyDownCallback(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_SetKeyUpCallback(SGTScript& caller, std::vector<SGTScriptParam> vParams);

	int FindParentWindow(int iSubWindowHandle);
	float m_fXPos, m_fYPos;
	int m_iHoverWin;
	bool m_bMenuActive;
	static const float m_fFactor;
	SGTScript m_CallbackScript;

	struct SWindowInfo
	{
		Ogre::String strName;
		Ogre::String strMaterial;
		float x, y, w, h;
		float fUScale, fVScale;
		int iDepth;
		bool bVisible;
		int iParentHandle;
		std::vector<int> vSubWindows;
		SGTScriptParam parOnClick, parMouseDown, parMouseUp, parMouseMove, parMouseHoverIn, parMouseHoverOut,
			parOnChar, parKeyDown, parKeyUp;
		bool bWasBaked;

	};
	
	std::map<int, SWindowInfo> m_mWindowInfos;
	std::list<int> m_lZOrder;
	int m_iCursorHandle;
	float m_fMaxZ;
	float m_fZStep;
	int m_iFocusWin;

	//scripting..
	std::map<std::string, SGTFontTextures> m_mFontTextures;
};

#endif