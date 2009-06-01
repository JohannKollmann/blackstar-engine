#ifndef __SGTGUISystem_H__
#define __SGTGUISystem_H__

#include "Ogre.h"
#include "SGTMessageListener.h"
#include "SGTMessageSystem.h"
#include "SGTMain.h"
#include "SGTSceneManager.h"
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


	void SetCursorMaterial(Ogre::String strMat);
	friend class SubWindow;
	friend class Window;
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
		static std::vector<SGTScriptParam> Lua_SetMaterial(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	private:
		std::vector<int> GetSubWindows();
		std::list<int> FindSubWindows(int iHandle);
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
	private:
		int m_iHandle;
	};
	Window MakeWindow(float x, float y, float w, float h, bool bPopUp=false);
	SGTGUISystem::SubWindow CreateSubWindow(float x, float y, float w, float h, int iParentHandle);
private:
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
		bool bPopUp;
		int iParentHandle;
		std::vector<int> vSubWindows;
		std::string strOnClickCallback, strHoverInCallback, strHoverOutCallback;
	};
	
	std::map<int, SWindowInfo> m_mWindowInfos;
	Window m_wMouse;
};

#endif