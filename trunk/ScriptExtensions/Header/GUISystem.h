#ifndef __GUISystem_H__
#define __GUISystem_H__

#include "Ogre.h"
#include "IceMessageListener.h"
#include "FontTextures.h"
#include <map>
#include <vector>
#include <list>
#include "IceScriptSystem.h"

class __declspec(dllexport) GUISystem : public Ice::MessageListener
{
public:
	GUISystem(void);
	static GUISystem& GetInstance(); 

	void ReceiveMessage(Ice::Msg &msg);

	class SubWindow;
	class Window;
	friend class GUISystem::SubWindow;
	friend class GUISystem::Window;

	class Window
	{
	public:
		Window(int iHandle);
		Window();
		void Bake();
		void SetMaterial(Ogre::String mat);
		void Move(float x, float y);
		int GetHandle();
		static std::list<int> FindSubWindows(int iHandle);
	private:
		std::vector<int> GetSubWindows();
		int m_iHandle;
	};
	class SubWindow
	{
	public:
		SubWindow(int iHandle);
		void SetMaterial(Ogre::String mat);
		int GetHandle();

	private:
		int m_iHandle;
	};
	GUISystem::Window MakeWindow(float x, float y, float w, float h, float fUScale=1.0f, float fVScale=1.0f);
	GUISystem::SubWindow CreateSubWindow(int iParentHandle, float x, float y, float w, float h, float fUScale=1.0f, float fVScale=1.0f);
	void SetFocus(int iHandle);
	void SetForegroundWindow(int iHandle);
	void SetVisible(int iHandle, bool bVisible);
	bool GetVisible(int iHandle);
	void SetCursor(int iHandle);
	void DeleteWindow(int iHandle);

	void Fade(int iHandle, float fFadeSpeed);

	void Clear();
private:
	//scripting stuff

	static std::vector<Ice::ScriptParam> Lua_CreateWindow(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_CreateSubWindow(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_BakeWindow(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_DeleteWindow(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);

	static std::vector<Ice::ScriptParam> Lua_SetMaterial(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_SetWindowVisible(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_FadeWindow(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_SetForegroundWindow(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_SetFocus(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_MoveWindow(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_GetWindowPos(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_SetCursorPos(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);

	static std::vector<Ice::ScriptParam> Lua_GetScreenCoords(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);

	static std::vector<Ice::ScriptParam> Lua_GetCursor(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_SetCursor(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);

	//font system
	static std::vector<Ice::ScriptParam> Lua_CreateFontMaterial(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_CreateFontTexture(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_ChangeFontMaterial(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_DeleteTexture(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_DeleteMaterial(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);

	//window callbacks
	//mouse events
	static std::vector<Ice::ScriptParam> Lua_SetOnClickCallback(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_SetMouseDownCallback(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_SetMouseUpCallback(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_SetMouseMoveCallback(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_SetMouseHoverInCallback(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_SetMouseHoverOutCallback(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	//keyboard events
	static std::vector<Ice::ScriptParam> Lua_SetOnCharCallback(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_SetKeyDownCallback(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_SetKeyUpCallback(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	//input states
	static std::vector<Ice::ScriptParam> Lua_GetKeyDown(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_GetMouseButtons(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);

	int FindParentWindow(int iSubWindowHandle);
	float m_fXPos, m_fYPos;
	int m_iHoverWin;
	static const float m_fFactor;

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
		Ice::ScriptParam parOnClick, parMouseDown, parMouseUp, parMouseMove, parMouseHoverIn, parMouseHoverOut,
			parOnChar, parKeyDown, parKeyUp;
		bool bWasBaked;
		float fFadeSpeed;
	};
	
	std::map<int, SWindowInfo> m_mWindowInfos;
	std::list<int> m_lZOrder;
	std::list<Ogre::String> m_lTextures;
	std::list<Ogre::String> m_lMaterials;
	int m_iCursorHandle;
	float m_fMaxZ;
	float m_fZStep;
	int m_iFocusWin;
	
	std::list<int> m_lFading;
	//std::map<int, EntityMaterialInstance*> m_mMatInstances;

	//scripting..
	std::map<std::string, FontTextures> m_mFontTextures;
};

#endif
