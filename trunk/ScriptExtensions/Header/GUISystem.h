#ifndef __GUISystem_H__
#define __GUISystem_H__

#include "Ogre.h"
#include "IceMessageListener.h"
#include "IceMain.h"
#include "FontTextures.h"
#include <map>
#include <vector>
#include <list>
#include "IceScriptSystem.h"

class __declspec(dllexport) GUISystem : public Ice::ViewMessageListener
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

	/**
	 * create a window
	 * @param x x-coordinate of window
	 * @param y y-coordinate of window
	 * @param w w-coordinate of window
	 * @param h h-coordinate of window
	 * 
	 * @return id id of the created window
	*/
	static std::vector<Ice::ScriptParam> Lua_CreateWindow(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	/**
	 * create a sub window
	 * @param parent parent window of the subwindow to create
	 * @param x x-coordinate of window
	 * @param y y-coordinate of window
	 * @param w w-coordinate of window
	 * @param h h-coordinate of window
	 * @param uscale scaling of window content in x direction (optional)
	 * @param vscale scaling of window content in y direction (optional)
	 * 
	 * @return id id of the created window
	*/
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
	/**
	 * create material for font texture
	 * @param spacing_file file containing font spacings for the used font
	 * @param texture_name name of the ogre texture
	 * @param base_material_name special material which will be copied by the system to make it use the font texture
	 * @param pass_name pass in base material to replace the texture in
	 * @param target_texture_name name of the texture to replace in base material
	 * @param maxwidth maxwidth parameter from {@link GUISystem::Lua_CreateFontTexture}
	 * @param maxheight maxheight parameter from {@link GUISystem::Lua_CreateFontTexture}
	 * parameters 8 through 13 are optional and define a 3x2 matrix used to transform the text
	 *
	 * @return material_name name of the created ogre material
	*/
	static std::vector<Ice::ScriptParam> Lua_CreateFontMaterial(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	/**
	 * create a compressed font texture
	 * @param spacing_file file containing font spacings for the used font
	 * @param text actual text for the texture
	 * @param maxwidth maximum width of the texture (in width units of spacing file)
	 * @param maxheight maximum height of the texture (in lines)
	 *
	 * @return texture_name name of the created ogre texture
	 * @return actual_width width of the ogre texture
	 * @return actual_height height of the ogre texture
	*/
	static std::vector<Ice::ScriptParam> Lua_CreateFontTexture(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	/**
	 * change material for font texture
	 * see {@link GUISystem::Lua_CreateFontMaterial}
	*/
	static std::vector<Ice::ScriptParam> Lua_ChangeFontMaterial(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	/**
	 * deletes ogre texture
	 * @param texture texture to delete
	*/
	static std::vector<Ice::ScriptParam> Lua_DeleteTexture(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	/**
	 * deletes ogre material
	 * @param material material to delete
	*/
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

	static void SetWindowPosition(Ogre::SubEntity* pSubEnt, float fX, float fY, bool bKeepRest=true);

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
