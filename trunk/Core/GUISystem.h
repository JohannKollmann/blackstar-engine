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

	class Window
	{
	public:
		Window(int iHandle);
		Window();
		void Bake();
		void SetMaterial(Ogre::String mat);
		void Move(float x, float y);
		int GetHandle();
		
		void SetOnClickCallback(std::string strCallback);
		void SetHoverInCallback(std::string strCallback);
		void SetHoverOutCallback(std::string strCallback);
		void SetKeyCallback(std::string strCallback);
	private:
		std::vector<int> GetSubWindows();
		static std::list<int> FindSubWindows(int iHandle);
		int m_iHandle;
	};
	class SubWindow
	{
	public:
		SubWindow(int iHandle);
		void SetMaterial(Ogre::String mat);
		int GetHandle();
	
		void SetOnClickCallback(std::string strCallback);
		void SetHoverInCallback(std::string strCallback);
		void SetHoverOutCallback(std::string strCallback);
		void SetKeyCallback(std::string strCallback);

	private:
		int m_iHandle;
	};
	SGTGUISystem::Window MakeWindow(float x, float y, float w, float h);
	SGTGUISystem::SubWindow CreateSubWindow(float x, float y, float w, float h, int iParentHandle);
	void SetFocus(int iHandle);
	void SetForegroundWindow(int iHandle);
	void SetVisible(int iHandle, bool bVisible);
	bool GetVisible(int iHandle);
	void SetCursor(int iHandle);


private:
	//scripting stuff

	static std::vector<SGTScriptParam> Lua_SetMaterial(SGTScript& caller, std::vector<SGTScriptParam> vParams);

	static std::vector<SGTScriptParam> Lua_CreateFontMaterial(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_CreateFontTexture(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_ChangeFontMaterial(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_DeleteTexture(SGTScript& caller, std::vector<SGTScriptParam> vParams);

	int FindParentWindow(int iSubWindowHandle);
	float m_fXPos, m_fYPos;
	int m_iHoverWin;
	bool m_bMenuActive;
	static const float m_fFactor;
	static const float cfFrameWidth;
	SGTScript m_CallbackScript;

	struct SWindowInfo
	{
		Ogre::String strName;
		float x, y, w, h;
		int iDepth;
		bool bVisible;
		int iParentHandle;
		std::vector<int> vSubWindows;
		std::string strOnClickCallback, strHoverInCallback, strHoverOutCallback, strKeyPressCallback;
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