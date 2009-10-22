#include "GUISystem.h"
#include <windows.h>
#include "SGTInput.h"
#include "SGTScriptableInstances.h"
#include "ResidentVariables.h"


SGTGUISystem::SGTGUISystem(void)
{
	SGTScriptableInstances::GetInstance();
	ResidentManager::GetInstance();
	m_iCursorHandle=-1;
	m_iFocusWin=-1;
	m_fMaxZ=0.0001f;
	m_fZStep=m_fMaxZ/1000.0f;

	SGTMessageSystem::Instance().JoinNewsgroup(this, "MOUSE_MOVE");
	SGTMessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
	SGTMessageSystem::Instance().JoinNewsgroup(this, "MOUSE_DOWN");
	SGTMessageSystem::Instance().JoinNewsgroup(this, "MOUSE_UP");
	SGTMessageSystem::Instance().JoinNewsgroup(this, "KEY_DOWN");
	Ogre::SceneNode* pNode=SGTMain::Instance().GetOgreSceneMgr()->getRootSceneNode()->createChildSceneNode("SGTGuiSystemNode");
	pNode->setPosition(0, 0, 0);
	m_fXPos=m_fYPos=0.5f;
	Window wMouse=MakeWindow(m_fXPos, m_fYPos, 0.05, 0.05);
	wMouse.Bake();
	wMouse.SetMaterial("gui/mouse");
	SetCursor(wMouse.GetHandle());
	
/*	Window wExitMenu=MakeWindow(0.25, 0.25, 0.5, 0.5);
	SGTGUISystem::SubWindow wContinue=CreateSubWindow(0.25, 0.12, 0.5, 0.20, wExitMenu.GetHandle());
	SGTGUISystem::SubWindow wStart=CreateSubWindow(0.25, 0.37, 0.5, 0.20, wExitMenu.GetHandle());
	SGTGUISystem::SubWindow wQuit=CreateSubWindow(0.25, 0.63, 0.5, 0.20, wExitMenu.GetHandle());

	SGTGUISystem::SubWindow wClear=CreateSubWindow(0.5, 0.8, 0.49, 0.19, wExitMenu.GetHandle());
	wExitMenu.Bake();
*/	
/*	SGTFontTextures ft("morpheus_spacings.txt");
	int iWidth, iHeight;
	//Ogre::MaterialPtr pMat=ft.CreateTextMaterial(ft.CreateTextTexture("$ff001$fff0N$f0f0S$f0ffA$f00fN$ff0fE$f0f0", 100, 3, iWidth, iHeight), "SubWindowFont", "TextPass", "TextTexture", 100, 3);
	Ogre::MaterialPtr pMat=ft.CreateTextMaterial(ft.CreateTextTexture("$f0f0YAY SGTGUISYSTEM MIT Z-ORDER!!", 100, 3, iWidth, iHeight), "SubWindowFont", "TextPass", "TextTexture", 100, 3);


	wExitMenu.SetMaterial("gui/win-test");
	wContinue.SetMaterial("gui/ResumeGame");
	wStart.SetMaterial("gui/StartGame");
	wQuit.SetMaterial("gui/QuitGame");

//	wExitMenu.SetMaterial(pMat->getName());
	wContinue.SetMaterial(pMat->getName());
	wStart.SetMaterial(pMat->getName());
	wQuit.SetMaterial(pMat->getName());
*/
	/*
	wContinue.SetHoverInCallback("ResumeHoverInCallback");
	wContinue.SetHoverOutCallback("ResumeHoverOutCallback");
	wContinue.SetOnClickCallback("ResumeClickCallback");

	wStart.SetHoverInCallback("StartHoverInCallback");
	wStart.SetHoverOutCallback("StartHoverOutCallback");
	wStart.SetOnClickCallback("StartClickCallback");

	wQuit.SetHoverInCallback("QuitHoverInCallback");
	wQuit.SetHoverOutCallback("QuitHoverOutCallback");
	wQuit.SetOnClickCallback("QuitClickCallback");
*/
	SGTScriptSystem::GetInstance().ShareCFunction("gui_create_window", Lua_CreateWindow);
	SGTScriptSystem::GetInstance().ShareCFunction("gui_create_subwindow", Lua_CreateSubWindow);
	SGTScriptSystem::GetInstance().ShareCFunction("gui_bake_window", Lua_BakeWindow);

	SGTScriptSystem::GetInstance().ShareCFunction("gui_set_window_material", Lua_SetMaterial);
	SGTScriptSystem::GetInstance().ShareCFunction("gui_set_visible", Lua_SetWindowVisible);
	SGTScriptSystem::GetInstance().ShareCFunction("gui_set_foreground_window", Lua_SetForegroundWindow);
	SGTScriptSystem::GetInstance().ShareCFunction("gui_set_focus", Lua_SetFocus);

	SGTScriptSystem::GetInstance().ShareCFunction("gui_get_screen_coords", Lua_GetScreenCoords);
	
	SGTScriptSystem::GetInstance().ShareCFunction("gui_get_cursor", Lua_GetCursor);
	SGTScriptSystem::GetInstance().ShareCFunction("gui_set_cursor", Lua_SetCursor);

	SGTScriptSystem::GetInstance().ShareCFunction("gui_change_font_material", Lua_ChangeFontMaterial);
	SGTScriptSystem::GetInstance().ShareCFunction("gui_create_font_material", Lua_CreateFontMaterial);
	SGTScriptSystem::GetInstance().ShareCFunction("gui_create_font_texture", Lua_CreateFontTexture);
	SGTScriptSystem::GetInstance().ShareCFunction("gui_delete_texture", Lua_DeleteTexture);

	//mouse events
	SGTScriptSystem::GetInstance().ShareCFunction("gui_set_on_click_callback", Lua_SetOnClickCallback);
	SGTScriptSystem::GetInstance().ShareCFunction("gui_set_mouse_down_callback", Lua_SetMouseDownCallback);
	SGTScriptSystem::GetInstance().ShareCFunction("gui_set_mouse_up_callback", Lua_SetMouseUpCallback);
	SGTScriptSystem::GetInstance().ShareCFunction("gui_set_mouse_move_callback", Lua_SetMouseMoveCallback);
	SGTScriptSystem::GetInstance().ShareCFunction("gui_set_mouse_hover_in_callback", Lua_SetMouseHoverInCallback);
	SGTScriptSystem::GetInstance().ShareCFunction("gui_set_mouse_hover_out_callback", Lua_SetMouseHoverOutCallback);
	//keyboard events
	SGTScriptSystem::GetInstance().ShareCFunction("gui_set_on_char_callback", Lua_SetOnCharCallback);
	SGTScriptSystem::GetInstance().ShareCFunction("gui_set_key_down_callback", Lua_SetKeyDownCallback);
	SGTScriptSystem::GetInstance().ShareCFunction("gui_set_key_up_callback", Lua_SetKeyUpCallback);

	m_bMenuActive=false;
}

SGTGUISystem::~SGTGUISystem(void)
{
}

void SGTGUISystem::ReceiveMessage(SGTMsg &msg)
{
	if(msg.mNewsgroup == "KEY_DOWN" || m_CallbackScript.GetID()==-1)
	{
		if(msg.mNewsgroup == "KEY_DOWN")
		{
			if(OIS::KC_ESCAPE==msg.mData.GetInt("KEY_ID_OIS"))
				m_bMenuActive=!m_bMenuActive;
			else
			{
				if(m_mWindowInfos.find(m_iFocusWin)->second.parKeyDown.getType()==SGTScriptParam::PARM_TYPE_FUNCTION && m_bMenuActive)
				{
					std::vector<SGTScriptParam> vResults;
					vResults.push_back(SGTScriptParam(m_iFocusWin));
					vResults.push_back(SGTScriptParam(msg.mData.GetInt("KEY_ID_OIS")));
					if(m_iFocusWin!=-1)
						SGTScriptSystem::RunCallbackFunction(m_mWindowInfos.find(m_iFocusWin)->second.parKeyDown, vResults);
				}
			}
		}
		std::map<int, SWindowInfo>::const_iterator it=m_mWindowInfos.begin();
		/*for(; it!=m_mWindowInfos.end(); it++)
			if(it->second.iParentHandle==-1)
				SGTMain::Instance().GetOgreSceneMgr()->getEntity(it->second.strName)->setVisible(m_bMenuActive);*/
		SGTMain::Instance().GetOgreSceneMgr()->getSceneNode("SGTGuiSystemNode")->setVisible(m_bMenuActive);
	}
	if(!m_bMenuActive)
		return;
	if (msg.mNewsgroup == "MOUSE_MOVE")
	{
		m_fXPos+=(float)msg.mData.GetInt("ROT_X_REL")/(float)SGTMain::Instance().GetViewport()->getActualWidth();
		m_fYPos+=(float)msg.mData.GetInt("ROT_Y_REL")/(float)SGTMain::Instance().GetViewport()->getActualHeight();
		//MessageBox(0, "", "", 0);
		if(m_fXPos>1.0)
			m_fXPos=1.0;
		if(m_fXPos<0.0)
			m_fXPos=0.0;
		if(m_fYPos>1.0)
			m_fYPos=1.0;
		if(m_fYPos<0.0)
			m_fYPos=0.0;
		//SGTMain::Instance().GetOgreSceneMgr()->getSceneNode("mySceneNode")->setPosition(m_fXPos, m_fYPos, 0);
		//SGTMain::Instance().GetOgreSceneMgr()->getEntity("bleh")->getSubEntity(0)->setCustomParameter(0, Ogre::Vector4(m_fXPos, m_fYPos, 0, 0));
		Window(m_iCursorHandle).Move(m_fXPos, m_fYPos);

		std::list<int>::iterator itZ=m_lZOrder.begin();
		std::map<int, SWindowInfo>::iterator it;
		for(; itZ!=m_lZOrder.end(); itZ++)
		{
			it=m_mWindowInfos.find(*itZ);
			if(m_fXPos>it->second.x && m_fXPos<it->second.x+it->second.w &&
				m_fYPos>it->second.y && m_fYPos<it->second.y+it->second.h && it->first!=m_iCursorHandle)
				break;
		}
		/*
		std::map<int, SWindowInfo>::iterator it=m_mWindowInfos.begin();
		for(; it!=m_mWindowInfos.end(); it++)
			if(m_fXPos>it->second.x && m_fXPos<it->second.x+it->second.w && it->second.iParentHandle==-1 &&
				m_fYPos>it->second.y && m_fYPos<it->second.y+it->second.h && it->first!=m_iCursorHandle)
				break;
		*/
		//mouse is somewhere inside this window, trace it down
		int iCurrHover=-1;
		if(it!=m_mWindowInfos.end())
		{
			float xoff=it->second.x;
			float yoff=it->second.y;
			for(unsigned int iSubWin=0; iSubWin<it->second.vSubWindows.size(); iSubWin++)
				if(m_fXPos>m_mWindowInfos.find(it->second.vSubWindows[iSubWin])->second.x+xoff && m_fXPos<m_mWindowInfos.find(it->second.vSubWindows[iSubWin])->second.x+xoff+m_mWindowInfos.find(it->second.vSubWindows[iSubWin])->second.w &&
					m_fYPos>m_mWindowInfos.find(it->second.vSubWindows[iSubWin])->second.y+yoff && m_fYPos<m_mWindowInfos.find(it->second.vSubWindows[iSubWin])->second.y+yoff+m_mWindowInfos.find(it->second.vSubWindows[iSubWin])->second.h)
				{
					it=m_mWindowInfos.find(it->second.vSubWindows[iSubWin]);
					iSubWin=-1;
				}
			iCurrHover=it->first;

			if(iCurrHover!=m_iHoverWin)
			{
				if(iCurrHover!=-1)
					if(it->second.parMouseHoverIn.getType()==SGTScriptParam::PARM_TYPE_FUNCTION)
					{
						std::vector<SGTScriptParam> parms(1, SGTScriptParam(it->first));
						SGTScriptSystem::RunCallbackFunction(it->second.parMouseHoverIn, parms);
					}
				if(m_iHoverWin!=-1)
					if(m_mWindowInfos.find(m_iHoverWin)->second.parMouseHoverOut.getType()==SGTScriptParam::PARM_TYPE_FUNCTION)
					{
						std::vector<SGTScriptParam> parms(1, m_iHoverWin);
						SGTScriptSystem::RunCallbackFunction(m_mWindowInfos.find(m_iHoverWin)->second.parMouseHoverOut, parms);
					}
			}
		}
		else
		{
			if(m_mWindowInfos.find(m_iHoverWin)!=m_mWindowInfos.end() && iCurrHover!=m_iHoverWin)
				if(m_mWindowInfos.find(m_iHoverWin)->second.parMouseHoverOut.getType()==SGTScriptParam::PARM_TYPE_FUNCTION)
				{
					std::vector<SGTScriptParam> parms(1, m_iHoverWin);
					SGTScriptSystem::RunCallbackFunction(m_mWindowInfos.find(m_iHoverWin)->second.parMouseHoverOut, parms);
				}
		}
		m_iHoverWin=iCurrHover;
	}
	if(msg.mNewsgroup == "UPDATE_PER_FRAME" && m_CallbackScript.GetID()==-1)
	{
		m_CallbackScript=SGTScriptSystem::GetInstance().CreateInstance("gui-hack.lua");
	}



	if(msg.mNewsgroup == "MOUSE_DOWN" || msg.mNewsgroup == "MOUSE_UP")
		if(OIS::MB_Left==msg.mData.GetInt("MOUSE_ID"))
		{
			/*std::map<int, SWindowInfo>::iterator it=m_mWindowInfos.begin();
			for(; it!=m_mWindowInfos.end(); it++)
				if(m_fXPos>it->second.x && m_fXPos<it->second.x+it->second.w &&
					m_fYPos>it->second.y && m_fYPos<it->second.y+it->second.h && it->first!=m_iCursorHandle)
					break;
				*/

			std::list<int>::iterator itZ=m_lZOrder.begin();
			std::map<int, SWindowInfo>::iterator it;
			for(; itZ!=m_lZOrder.end(); itZ++)
			{
				it=m_mWindowInfos.find(*itZ);
				if(m_fXPos>it->second.x && m_fXPos<it->second.x+it->second.w &&
					m_fYPos>it->second.y && m_fYPos<it->second.y+it->second.h && it->first!=m_iCursorHandle)
					break;
			}

			//mouse is somewhere inside this window, trace it down
			if(it!=m_mWindowInfos.end())
			{
				float xoff=it->second.x;
				float yoff=it->second.y;
				for(unsigned int iSubWin=0; iSubWin<it->second.vSubWindows.size(); iSubWin++)
					if(m_fXPos>m_mWindowInfos.find(it->second.vSubWindows[iSubWin])->second.x+xoff && m_fXPos<m_mWindowInfos.find(it->second.vSubWindows[iSubWin])->second.x+xoff+m_mWindowInfos.find(it->second.vSubWindows[iSubWin])->second.w &&
						m_fYPos>m_mWindowInfos.find(it->second.vSubWindows[iSubWin])->second.y+yoff && m_fYPos<m_mWindowInfos.find(it->second.vSubWindows[iSubWin])->second.y+yoff+m_mWindowInfos.find(it->second.vSubWindows[iSubWin])->second.h)
					{
						it=m_mWindowInfos.find(it->second.vSubWindows[iSubWin]);
						iSubWin=-1;
					}

				/*if(it->second.parMouseHoverOut.getType()==SGTScriptParam::PARM_TYPE_FUNCTION)
				{
					std::vector<SGTScriptParam> parms(1, m_iHoverWin);
					SGTScriptSystem::RunCallbackFunction(it->second.parMouseHoverOut, parms);
				}*/

				std::vector<SGTScriptParam> parms(1, SGTScriptParam(it->first));
				if(it->second.parMouseDown.getType()==SGTScriptParam::PARM_TYPE_FUNCTION && msg.mNewsgroup=="MOUSE_DOWN")
					SGTScriptSystem::RunCallbackFunction(it->second.parMouseDown, parms);
				if(it->second.parMouseUp.getType()==SGTScriptParam::PARM_TYPE_FUNCTION && msg.mNewsgroup=="MOUSE_UP")
					SGTScriptSystem::RunCallbackFunction(it->second.parMouseUp, parms);

			}
			//send it to the cursor itself, too
			it=m_mWindowInfos.find(m_iCursorHandle);
			std::vector<SGTScriptParam> parms(1, m_iCursorHandle);
			if(it!=m_mWindowInfos.end())
			{
				if(it->second.parMouseDown.getType()==SGTScriptParam::PARM_TYPE_FUNCTION && msg.mNewsgroup=="MOUSE_DOWN")
					SGTScriptSystem::RunCallbackFunction(it->second.parMouseDown, parms);
				if(it->second.parMouseUp.getType()==SGTScriptParam::PARM_TYPE_FUNCTION && msg.mNewsgroup=="MOUSE_UP")
					SGTScriptSystem::RunCallbackFunction(it->second.parMouseUp, parms);
			}
		}	
}

SGTGUISystem&
SGTGUISystem::GetInstance()
{
	static SGTGUISystem singleton;
	return singleton;
}

SGTGUISystem::Window
SGTGUISystem::MakeWindow(float x, float y, float w, float h, float fUScale, float fVScale)
{
	int iHandle=SGTSceneManager::Instance().RequestID();
	Window win(iHandle);
	SWindowInfo wininfo;
	wininfo.iParentHandle=-1;
	wininfo.strName=Ogre::String("GUI-System-Window ") + SGTSceneManager::Instance().RequestIDStr();
	wininfo.x=x;
	wininfo.y=y;
	wininfo.w=w;
	wininfo.h=h;
	wininfo.fUScale=fUScale;
	wininfo.fVScale=fVScale;
	wininfo.iDepth=1;
	wininfo.bWasBaked=false;
	m_mWindowInfos.insert(std::make_pair<int, SWindowInfo>(iHandle, wininfo));
	return Window(iHandle);
}


void
SGTGUISystem::Window::Bake()
{
	SGTGUISystem::SWindowInfo wininfo=GetInstance().m_mWindowInfos.find(m_iHandle)->second;
	float x=wininfo.x, y=wininfo.y, w=wininfo.w, h=wininfo.h, u=wininfo.fUScale, v=wininfo.fVScale;
	Ogre::MeshPtr meshptr = Ogre::MeshManager::getSingleton().createManual(wininfo.strName, "General");
	meshptr.get()->createSubMesh("windowface");

	std::vector<int> vSubWindows=GetSubWindows();
	int nSubWindows=vSubWindows.size();
	int nTris=(nSubWindows+1)*2;
	int nVerts=(nSubWindows+1)*4;
	Ogre::VertexData* data = new Ogre::VertexData();
	meshptr.get()->sharedVertexData = data;
	data->vertexCount = nVerts;
	Ogre::VertexDeclaration* decl = data->vertexDeclaration;
	decl->addElement(0, 0, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
	size_t off = Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
	decl->addElement(0, off, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES);
	Ogre::HardwareVertexBufferSharedPtr vbuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
	decl->getVertexSize(0), nVerts, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);

	float* afVertexData=(float*)vbuf->lock(Ogre::HardwareBuffer::HBL_DISCARD);
	afVertexData[0]=0.0f;
	afVertexData[1]=0.0f;
	afVertexData[2]=0.0f;
	afVertexData[3]=0.0f;
	afVertexData[4]=0.0f;

	afVertexData[5]=w;
	afVertexData[6]=0.0f;
	afVertexData[7]=0.0f;
	afVertexData[8]=u;
	afVertexData[9]=0.0f;

	afVertexData[10]=w;
	afVertexData[11]=h;
	afVertexData[12]=0.0f;
	afVertexData[13]=u;
	afVertexData[14]=v;

	afVertexData[15]=0.0f;
	afVertexData[16]=h;
	afVertexData[17]=0.0f;
	afVertexData[18]=0.0f;
	afVertexData[19]=v;
	vbuf->unlock();

	//now the sub-windows
	int iSubWin=0;
	for(iSubWin=0; iSubWin<nSubWindows; iSubWin++)
	{
		SGTGUISystem::SWindowInfo subwininfo=SGTGUISystem::GetInstance().m_mWindowInfos.find(vSubWindows[iSubWin])->second;
		//get z level of this sub-window
		int iLevel=0;
		SGTGUISystem::SWindowInfo tempinfo=subwininfo;
		while(tempinfo.iParentHandle!=-1)
		{
			tempinfo=SGTGUISystem::GetInstance().m_mWindowInfos.find(tempinfo.iParentHandle)->second;
			iLevel++;
		}
		if(iLevel>wininfo.iDepth)
			wininfo.iDepth=iLevel;
		//while(tempinfo
		float z=-SGTGUISystem::GetInstance().m_fZStep*(float)iLevel;
		float afSubWinVerts[]={
		subwininfo.x,subwininfo.y,z,0.0,0.0,
		subwininfo.x+subwininfo.w,subwininfo.y,z,subwininfo.fUScale,0.0,
		subwininfo.x,subwininfo.y+subwininfo.h,z,0.0,subwininfo.fVScale,
		subwininfo.x+subwininfo.w,subwininfo.y+subwininfo.h,z,subwininfo.fUScale,subwininfo.fVScale};
		vbuf->writeData((iSubWin+1)*4*decl->getVertexSize(0), 4*decl->getVertexSize(0), afSubWinVerts, false);
	}
	GetInstance().m_mWindowInfos.find(m_iHandle)->second.iDepth=wininfo.iDepth+1;

	Ogre::VertexBufferBinding* bind = data->vertexBufferBinding;
	bind->setBinding(0, vbuf);

	Ogre::HardwareIndexBufferSharedPtr ibuf = Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
	Ogre::HardwareIndexBuffer::IT_16BIT, nTris*3, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
			
	unsigned short* aiIndexBuf=(unsigned short*)ibuf->lock(Ogre::HardwareBuffer::HBL_DISCARD);
	aiIndexBuf[0]=0;
	aiIndexBuf[1]=1;
	aiIndexBuf[2]=2;
	aiIndexBuf[3]=0;
	aiIndexBuf[4]=2;
	aiIndexBuf[5]=3;
	ibuf->unlock();

	for(iSubWin=0; iSubWin<nSubWindows; iSubWin++)
	{
		SWindowInfo subwininfo=SGTGUISystem::GetInstance().m_mWindowInfos.find(vSubWindows[iSubWin])->second;
		unsigned short aiSubWinIndices[]={
			4*(iSubWin+1), 4*(iSubWin+1)+1, 4*(iSubWin+1)+2,
			4*(iSubWin+1)+1, 4*(iSubWin+1)+3, 4*(iSubWin+1)+2,
		};
		ibuf->writeData(sizeof(unsigned short)*(6*(iSubWin+1)), 6*sizeof(unsigned short), aiSubWinIndices, false);
		Ogre::SubMesh* pSM=meshptr.get()->createSubMesh(subwininfo.strName);
		pSM->indexData->indexBuffer = ibuf;
		pSM->indexData->indexCount = 2*3;
		pSM->indexData->indexStart = 2*3*(iSubWin+1);
	}

	meshptr.get()->getSubMesh("windowface")->indexData->indexBuffer = ibuf;
	meshptr.get()->getSubMesh("windowface")->indexData->indexCount = 2*3;
	meshptr.get()->getSubMesh("windowface")->indexData->indexStart = 0;
	float fInf=1000000000.0f;
	meshptr.get()->_setBounds(Ogre::AxisAlignedBox(-fInf,-fInf,-fInf,fInf,fInf,fInf));
	meshptr.get()->_setBoundingSphereRadius(fInf);		
	meshptr.get()->load();
	SGTMain::Instance().GetOgreSceneMgr()->getSceneNode("SGTGuiSystemNode")->attachObject(SGTMain::Instance().GetOgreSceneMgr()->createEntity(wininfo.strName, wininfo.strName));

	GetInstance().m_mWindowInfos.find(m_iHandle)->second.bWasBaked=true;

	SGTMain::Instance().GetOgreSceneMgr()->getEntity(wininfo.strName)->getSubEntity(0)->setCustomParameter(0, Ogre::Vector4(x, y, 0, 0));
	if(wininfo.strMaterial.size())
		SGTMain::Instance().GetOgreSceneMgr()->getEntity(wininfo.strName)->getSubEntity(0)->setMaterialName(wininfo.strMaterial);
	for(iSubWin=0; iSubWin<nSubWindows; iSubWin++)
	{
		SWindowInfo subwininfo=GetInstance().m_mWindowInfos.find(vSubWindows[iSubWin])->second;
		Ogre::SubEntity* pSubEnt=SGTMain::Instance().GetOgreSceneMgr()->getEntity(wininfo.strName)->getSubEntity(subwininfo.strName);
		pSubEnt->setCustomParameter(0, Ogre::Vector4(x, y, 0, 0));
		if(wininfo.strMaterial.size())
			pSubEnt->setMaterialName(subwininfo.strMaterial);
	}
	SGTGUISystem::GetInstance().SetForegroundWindow(m_iHandle);
	SGTGUISystem::GetInstance().SetFocus(m_iHandle);
}

std::list<int>
SGTGUISystem::Window::FindSubWindows(int iHandle)
{
	std::vector<int> vSubWins=SGTGUISystem::GetInstance().m_mWindowInfos.find(iHandle)->second.vSubWindows;
	std::list<int> lSubWins;
	unsigned int iSubWin=0;
	for(; iSubWin<vSubWins.size(); iSubWin++)
		lSubWins.push_back(vSubWins[iSubWin]);
	std::list<int> lTemp;
	for(iSubWin=0; iSubWin<vSubWins.size(); iSubWin++)
	{
		lTemp=FindSubWindows(vSubWins[iSubWin]);	
		for(std::list<int>::iterator it=lTemp.begin(); it!=lTemp.end(); it++)
			lSubWins.push_back(*it);
	}
	return lSubWins;
}

std::vector<int>
SGTGUISystem::Window::GetSubWindows()
{
	std::vector<int> vRes;
	std::list<int> lSubWins=FindSubWindows(m_iHandle);
	for(std::list<int>::iterator it=lSubWins.begin(); it!=lSubWins.end(); it++)
		vRes.push_back(*it);
	return vRes;
}


void
SGTGUISystem::Window::SetMaterial(Ogre::String strMat)
{
	std::map<int, SWindowInfo>::iterator it=GetInstance().m_mWindowInfos.find(m_iHandle);
	it->second.strMaterial=strMat;
	if(it->second.bWasBaked)
		SGTMain::Instance().GetOgreSceneMgr()->getEntity(it->second.strName)->getSubEntity("windowface")->setMaterialName(strMat);
}

void
SGTGUISystem::SubWindow::SetMaterial(Ogre::String strMat)
{
	GetInstance().m_mWindowInfos.find(m_iHandle)->second.strMaterial=strMat;
	SGTGUISystem::SWindowInfo wininfo=GetInstance().m_mWindowInfos.find(m_iHandle)->second;
	Ogre::String strSubMeshName=wininfo.strName;
	while(wininfo.iParentHandle!=-1)
		wininfo=SGTGUISystem::GetInstance().m_mWindowInfos.find(wininfo.iParentHandle)->second;
	//Ogre::Mesh* pMesh=Ogre::MeshManager::getSingleton().getByName(wininfo.strName);
	///pMesh->getSubMesh(strSubMeshName)->setMaterialName(strMat);
	if(wininfo.bWasBaked)
		SGTMain::Instance().GetOgreSceneMgr()->getEntity(wininfo.strName)->getSubEntity(strSubMeshName)->setMaterialName(strMat);//getMesh()->getSubMesh(strSubMeshName)->setMaterialName(strMat);
}


SGTGUISystem::SubWindow
SGTGUISystem::CreateSubWindow(int iParentHandle, float x, float y, float w, float h, float fUScale, float fVScale)
{
	int iHandle=SGTSceneManager::Instance().RequestID();
	m_mWindowInfos.find(iParentHandle)->second.vSubWindows.push_back(iHandle);
	SWindowInfo parenthandle=m_mWindowInfos.find(iParentHandle)->second;
	SWindowInfo wininfo;
	wininfo.x=x*parenthandle.w + ((parenthandle.iParentHandle!=-1) ? parenthandle.x : 0);
	wininfo.y=y*parenthandle.h + ((parenthandle.iParentHandle!=-1) ? parenthandle.y : 0);
	wininfo.w=w*parenthandle.w;
	wininfo.h=h*parenthandle.h;
	wininfo.fUScale=fUScale;
	wininfo.fVScale=fVScale;
	wininfo.iParentHandle=iParentHandle;
	wininfo.strName=Ogre::String("GUI-System-Subwindow ") + SGTSceneManager::Instance().RequestIDStr();
	m_mWindowInfos.insert(std::make_pair<int, SWindowInfo>(iHandle, wininfo));
	return SubWindow(iHandle);
}

SGTGUISystem::Window::Window(int iHandle){m_iHandle=iHandle;}
SGTGUISystem::Window::Window(){m_iHandle=-1;}
SGTGUISystem::SubWindow::SubWindow(int iHandle){m_iHandle=iHandle;}

void
SGTGUISystem::Window::Move(float x, float y)
{
	GetInstance().m_mWindowInfos.find(m_iHandle)->second.x=x;
	GetInstance().m_mWindowInfos.find(m_iHandle)->second.y=y;
	SWindowInfo wi=GetInstance().m_mWindowInfos.find(m_iHandle)->second;
	Ogre::Entity* pEnt=SGTMain::Instance().GetOgreSceneMgr()->getEntity(wi.strName);
	for(unsigned int iSubEnt=0; iSubEnt<pEnt->getNumSubEntities(); iSubEnt++)
	{
		Ogre::Vector4 vCustom=pEnt->getSubEntity(iSubEnt)->getCustomParameter(0);
		vCustom.x=x;
		vCustom.y=y;
		pEnt->getSubEntity(iSubEnt)->setCustomParameter(0, vCustom);
	}
}

int
SGTGUISystem::Window::GetHandle()
{
	return m_iHandle;
}

int
SGTGUISystem::SubWindow::GetHandle()
{
	return m_iHandle;
}

void
SGTGUISystem::SetForegroundWindow(int iHandle)
{
	//get window for subwindow
	SWindowInfo wininfo=m_mWindowInfos.find(iHandle)->second;
	while(wininfo.iParentHandle!=-1)
		wininfo=SGTGUISystem::GetInstance().m_mWindowInfos.find(wininfo.iParentHandle)->second;

	if(m_iCursorHandle!=iHandle)
	{
		//m_lZOrder.remove(iHandle);
		for(std::list<int>::iterator it=m_lZOrder.begin(); it!=m_lZOrder.end(); it++)
		{
			if(*it==iHandle)
			{
				m_lZOrder.erase(it);
				it=m_lZOrder.begin();
			}
		}
	}
	if(m_iCursorHandle!=-1)
		m_lZOrder.pop_front();
	m_lZOrder.push_front(iHandle);
	if(m_iCursorHandle!=-1 && m_iCursorHandle!=iHandle)
		m_lZOrder.push_front(m_iCursorHandle);
	int iCurrDepth=0;
	for(std::list<int>::iterator it=m_lZOrder.begin(); it!=m_lZOrder.end(); it++)
	{
		wininfo=m_mWindowInfos.find(*it)->second;
		iCurrDepth+=wininfo.iDepth;
		Ogre::Entity* pEnt=SGTMain::Instance().GetOgreSceneMgr()->getEntity(wininfo.strName);
		for(unsigned int iSubEnt=0; iSubEnt<pEnt->getNumSubEntities(); iSubEnt++)
		{
			Ogre::Vector4 vCustom=pEnt->getSubEntity(iSubEnt)->getCustomParameter(0);
			vCustom.z=(float)iCurrDepth*m_fZStep;
			pEnt->getSubEntity(iSubEnt)->setCustomParameter(0, vCustom);
		}
	}
}

void
SGTGUISystem::SetFocus(int iHandle)
{
	m_iFocusWin=iHandle;
}

int
SGTGUISystem::FindParentWindow(int iSubWindowHandle)
{
	SGTGUISystem::SWindowInfo subwininfo=m_mWindowInfos.find(iSubWindowHandle)->second;
	int iHandle=iSubWindowHandle;
	while(subwininfo.iParentHandle!=-1)
	{
		iHandle=subwininfo.iParentHandle;
		subwininfo=m_mWindowInfos.find(subwininfo.iParentHandle)->second;
	}
	return iHandle;
}


void
SGTGUISystem::SetVisible(int iHandle, bool bVisible)
{
	SWindowInfo subwininfo=m_mWindowInfos.find(iHandle)->second;
	m_mWindowInfos.find(iHandle)->second.bVisible=bVisible;
	if(subwininfo.iParentHandle==-1)
	{//hide a window, thats quite easy
		SGTMain::Instance().GetOgreSceneMgr()->getEntity(subwininfo.strName)->setVisible(bVisible);
		return;
	}
	//hiding subwindows requires more work
	SWindowInfo wininfo=m_mWindowInfos.find(FindParentWindow(iHandle))->second;
	Ogre::Vector4 vCustom=SGTMain::Instance().GetOgreSceneMgr()->getEntity(wininfo.strName)->getSubEntity(subwininfo.strName)->getCustomParameter(0);
	vCustom.x=bVisible ? wininfo.x : 10;
	SGTMain::Instance().GetOgreSceneMgr()->getEntity(wininfo.strName)->getSubEntity(subwininfo.strName)->setCustomParameter(0, vCustom);
}

bool
SGTGUISystem::GetVisible(int iHandle)
{
	return m_mWindowInfos.find(iHandle)->second.bVisible;
}

void
SGTGUISystem::SetCursor(int iHandle)
{
	if(m_mWindowInfos.find(iHandle)->second.iParentHandle!=-1)
		return;
	m_iCursorHandle=iHandle;
	m_lZOrder.remove(iHandle);
	m_lZOrder.push_front(iHandle);
	SetForegroundWindow(iHandle);
}

std::string GetTypeName(SGTScriptParam param)
{
	switch(param.getType())
	{
	case SGTScriptParam::PARM_TYPE_BOOL:
		return std::string("bool");
	case SGTScriptParam::PARM_TYPE_FLOAT:
		return std::string("float");
	case SGTScriptParam::PARM_TYPE_FUNCTION:
		return std::string("function");
	case SGTScriptParam::PARM_TYPE_INT:
		return std::string("int");
	case SGTScriptParam::PARM_TYPE_STRING:
		return std::string("string");
	default:
		return std::string("unknown type");
	}
}

std::string
TestParameters(std::vector<SGTScriptParam> testparams, std::vector<SGTScriptParam> refparams, bool bAllowMore)
{
	std::string strOut("");
	//test the number of parameters
	if(testparams.size()<refparams.size() || (testparams.size()>refparams.size() && !bAllowMore))
	{
		std::stringstream sstr;
		std::string strNum;
		std::string strErr=std::string("expecting ") + (bAllowMore ? std::string("at least ") : std::string(""));
		sstr<<refparams.size();
		sstr>>strNum;
		sstr.clear();
		strErr=strErr + strNum + std::string(" , not ");
		sstr<<testparams.size();
		sstr>>strNum;
		strErr=strErr + strNum + std::string("! ");
		strOut=strOut + strErr;
	}
	//test every single parameter
	for(unsigned int iParam=0; iParam<((refparams.size()<testparams.size()) ? refparams.size() : testparams.size()) ; iParam++)
	{
		if(refparams[iParam].getType()!=testparams[iParam].getType())
		{
			std::stringstream sstr;
			std::string strErr;
			sstr<<iParam;
			sstr>>strErr;
			strErr=std::string("expecting parameter ") + strErr + std::string(" to be of type ") + GetTypeName(refparams[iParam]) + std::string(", not ") + GetTypeName(testparams[iParam]) + std::string("! ");
			strOut+=strErr;
		}
	}
	return strOut;
}

std::vector<SGTScriptParam>
SGTGUISystem::Lua_CreateFontTexture(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{//arguments: spacing file, text, maxwidth, maxheight
//outputs: texture name, actual width, actual height
	std::map<std::string, SGTFontTextures>::iterator it=GetInstance().m_mFontTextures.find(vParams[0].getString());
	if(it==GetInstance().m_mFontTextures.end())
	{//we don't have this spacing in memory, load it
		GetInstance().m_mFontTextures.insert(std::pair<std::string, SGTFontTextures>(vParams[0].getString(), SGTFontTextures(vParams[0].getString())));
		it=GetInstance().m_mFontTextures.find(vParams[0].getString());
	}
	//use that font textures object to crate the texture
	int iActualWidth, iActualHeight;
	std::vector<SGTScriptParam> vRes;
	Ogre::TexturePtr pTex=it->second.CreateTextTexture(vParams[1].getString(), (int)vParams[2].getFloat(), (int)vParams[3].getFloat(), iActualWidth, iActualHeight);
	vRes.push_back(SGTScriptParam(pTex->getName()));
	vRes.push_back(SGTScriptParam(iActualWidth));
	vRes.push_back(SGTScriptParam(iActualHeight));
	return vRes;
}

std::vector<SGTScriptParam>
SGTGUISystem::Lua_CreateFontMaterial(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{//arguments: spacing file, texture name, base material name, pass name, target texture name, maxwidth, maxheight
//outputs: material name
	std::map<std::string, SGTFontTextures>::iterator it=GetInstance().m_mFontTextures.find(vParams[0].getString());
	if(it==GetInstance().m_mFontTextures.end())
	{//we don't have this spacing in memory, load it
		GetInstance().m_mFontTextures.insert(std::pair<std::string, SGTFontTextures>(vParams[0].getString(), SGTFontTextures(vParams[0].getString())));
		it=GetInstance().m_mFontTextures.find(vParams[0].getString());
	}
	Ogre::MaterialPtr pMat=it->second.CreateTextMaterial(Ogre::TextureManager::getSingleton().getByName(vParams[1].getString()), vParams[2].getString(), vParams[3].getString(), vParams[4].getString(), (int)vParams[5].getFloat(), (int)vParams[6].getFloat());
	return std::vector<SGTScriptParam>(1, SGTScriptParam(pMat->getName()));
}

std::vector<SGTScriptParam>
SGTGUISystem::Lua_ChangeFontMaterial(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{//arguments: spacing file, material name, texture name, pass name, target texture name, maxwidth, maxheight
//outputs: none
	std::map<std::string, SGTFontTextures>::iterator it=GetInstance().m_mFontTextures.find(vParams[0].getString());
	if(it==GetInstance().m_mFontTextures.end())
	{//we don't have this spacing in memory, load it
		GetInstance().m_mFontTextures.insert(std::pair<std::string, SGTFontTextures>(vParams[0].getString(), SGTFontTextures(vParams[0].getString())));
		it=GetInstance().m_mFontTextures.find(vParams[0].getString());
	}
	it->second.ChangeTextMaterial(
		Ogre::MaterialManager::getSingleton().getByName(vParams[1].getString()),
		Ogre::TextureManager::getSingleton().getByName(vParams[2].getString()),
		vParams[3].getString(),
		vParams[4].getString(),
		(int)vParams[5].getFloat(),
		(int)vParams[6].getFloat());

	return std::vector<SGTScriptParam>();
}

std::vector<SGTScriptParam>
SGTGUISystem::Lua_DeleteTexture(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{//arguments: texture name
	Ogre::TextureManager::getSingleton().remove(vParams[0].getString());
	return std::vector<SGTScriptParam>();
}

std::vector<SGTScriptParam>
SGTGUISystem::Lua_CreateWindow(SGTScript &caller, std::vector<SGTScriptParam> vParams)
{//input: x,y,w,h
//return: window-id
	std::vector<SGTScriptParam> ret;
	if(vParams.size()!=4)
		return ret;
	if(vParams[0].getType()!=SGTScriptParam::PARM_TYPE_FLOAT ||
		vParams[1].getType()!=SGTScriptParam::PARM_TYPE_FLOAT ||
		vParams[2].getType()!=SGTScriptParam::PARM_TYPE_FLOAT ||
		vParams[3].getType()!=SGTScriptParam::PARM_TYPE_FLOAT)
		return ret;
	ret.push_back(SGTGUISystem::GetInstance().MakeWindow(vParams[0].getFloat(), vParams[1].getFloat(), vParams[2].getFloat(), vParams[3].getFloat()).GetHandle());
	return ret;
}

std::vector<SGTScriptParam>
SGTGUISystem::Lua_CreateSubWindow(SGTScript &caller, std::vector<SGTScriptParam> vParams)
{//input: x,y,w,h,parent
//return: window-id
	std::vector<SGTScriptParam> ret;
	for(int iParm=0; iParm<5; iParm++)
		ret.push_back(SGTScriptParam(0.0));
	std::string strError=TestParameters(vParams, ret, true);
	ret.clear();
	ret.push_back(SGTScriptParam());
	if(strError.size())
	{
		ret.push_back(SGTScriptParam(strError));
		return ret;
	}
	if(vParams.size()!=5 && vParams.size()!=7)
	{
		ret.push_back(SGTScriptParam(std::string("function expects 5 or 7 parameters")));
		return ret;
	}
	if(vParams.size()==7)
		if(vParams[5].getType()!=SGTScriptParam::PARM_TYPE_FLOAT ||
			vParams[6].getType()!=SGTScriptParam::PARM_TYPE_FLOAT)
		{
			ret.push_back(SGTScriptParam(std::string("parameters 5 and 6 have to be floats")));
			return ret;
		}
	
	ret.clear();
	if(vParams.size()==5)
		ret.push_back(GetInstance().CreateSubWindow((int)vParams[0].getFloat(), vParams[1].getFloat(), vParams[2].getFloat(), vParams[3].getFloat(), vParams[4].getFloat()).GetHandle());
	else
		ret.push_back(GetInstance().CreateSubWindow((int)vParams[0].getFloat(), vParams[1].getFloat(), vParams[2].getFloat(), vParams[3].getFloat(), vParams[4].getFloat(), vParams[5].getFloat(), vParams[6].getFloat()).GetHandle());
	return ret;
}

std::vector<SGTScriptParam>
SGTGUISystem::Lua_BakeWindow(SGTScript &caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> ret;
	if(vParams.size()!=1)
		return ret;
	if(vParams[0].getType()!=SGTScriptParam::PARM_TYPE_FLOAT)
		return ret;
	Window((int)vParams[0].getFloat()).Bake();
	return ret;
}

std::vector<SGTScriptParam>
SGTGUISystem::Lua_SetMaterial(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> ret;
	if(vParams.size()!=2)
		return ret;
	if(vParams[0].getType()!=SGTScriptParam::PARM_TYPE_FLOAT || vParams[1].getType()!=SGTScriptParam::PARM_TYPE_STRING)
		return ret;
	SWindowInfo wininfo=SGTGUISystem::GetInstance().m_mWindowInfos.find((int)vParams[0].getFloat())->second;
	if(wininfo.iParentHandle==-1)//is top-level-window
		Window((int)vParams[0].getFloat()).SetMaterial(vParams[1].getString());
	else
		SubWindow((int)vParams[0].getFloat()).SetMaterial(vParams[1].getString());

	return ret;
}

std::vector<SGTScriptParam>
SGTGUISystem::Lua_SetWindowVisible(SGTScript &caller, std::vector<SGTScriptParam> vParams)
{//inputs: window-id,bool
	std::vector<SGTScriptParam> ret;
	if(vParams.size()!=2)
		return ret;
	if(vParams[0].getType()!=SGTScriptParam::PARM_TYPE_FLOAT ||
		vParams[1].getType()!=SGTScriptParam::PARM_TYPE_BOOL)
		return ret;
	SGTGUISystem::GetInstance().SetVisible((int)vParams[0].getFloat(), vParams[1].getBool());
	return ret;
}
std::vector<SGTScriptParam>
SGTGUISystem::Lua_SetForegroundWindow(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> errout;
	errout.push_back(SGTScriptParam());
	std::string strErrString=TestParameters(vParams, std::vector<SGTScriptParam>(1, SGTScriptParam(0.1)), false);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	if(GetInstance().m_mWindowInfos.find((int)vParams[0].getFloat())==GetInstance().m_mWindowInfos.end())
	{
		errout.push_back(SGTScriptParam(std::string("could not find a window with given ID")));
		return errout;
	}
	GetInstance().SetForegroundWindow((int)vParams[0].getFloat());
	return std::vector<SGTScriptParam>();
}

std::vector<SGTScriptParam>
SGTGUISystem::Lua_SetFocus(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> errout;
	errout.push_back(SGTScriptParam());
	std::string strErrString=TestParameters(vParams, std::vector<SGTScriptParam>(1, SGTScriptParam(0.1)), false);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	if(GetInstance().m_mWindowInfos.find((int)vParams[0].getFloat())==GetInstance().m_mWindowInfos.end())
	{
		errout.push_back(SGTScriptParam(std::string("could not find a window with given ID")));
		return errout;
	}
	GetInstance().SetFocus((int)vParams[0].getFloat());
	return std::vector<SGTScriptParam>();
}

std::vector<SGTScriptParam>
SGTGUISystem::Lua_GetScreenCoords(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> errout;
	errout.push_back(SGTScriptParam());
	std::string strErrString=TestParameters(vParams, std::vector<SGTScriptParam>(1, SGTScriptParam(0.1)), false);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	if(GetInstance().m_mWindowInfos.find((int)vParams[0].getFloat())==GetInstance().m_mWindowInfos.end())
	{
		errout.push_back(SGTScriptParam(std::string("could not find a window with given ID")));
		return errout;
	}
	SWindowInfo wi=GetInstance().m_mWindowInfos.find((int)vParams[0].getFloat())->second;
	errout.clear();
	errout.push_back(SGTScriptParam(wi.x));
	errout.push_back(SGTScriptParam(wi.y));
	errout.push_back(SGTScriptParam(wi.w));
	errout.push_back(SGTScriptParam(wi.h));
	return errout;
}

std::vector<SGTScriptParam>
SGTGUISystem::Lua_GetCursor(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	return std::vector<SGTScriptParam>(1, SGTScriptParam(GetInstance().m_iCursorHandle));
}

std::vector<SGTScriptParam> SGTGUISystem::Lua_SetCursor(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> errout;
	errout.push_back(SGTScriptParam());
	std::string strErrString=TestParameters(vParams, std::vector<SGTScriptParam>(1, SGTScriptParam(0.1)), false);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	if(GetInstance().m_mWindowInfos.find((int)vParams[0].getFloat())==GetInstance().m_mWindowInfos.end())
	{
		errout.push_back(SGTScriptParam(std::string("could not find a window with given ID")));
		return errout;
	}
	GetInstance().SetCursor((int)vParams[0].getFloat());
	return std::vector<SGTScriptParam>();
}



//a little macro for the callback-setters. it will test the parameters and generate meaningful error texts
//then set the given field in the window infos
#define LUA_CALLBACK_SETTER_MACRO(params, wininfofield, script)\
	std::vector<SGTScriptParam> errout(1, SGTScriptParam());\
	std::vector<SGTScriptParam> vRef(1, SGTScriptParam(0.1));\
	vRef.push_back(SGTScriptParam("", script));\
	std::string strErrString=TestParameters(vParams, vRef, false);\
	if(strErrString.length())\
		{errout.push_back(strErrString);return errout;}\
	if(GetInstance().m_mWindowInfos.find((int)params[0].getFloat())==GetInstance().m_mWindowInfos.end())\
		{errout.push_back(SGTScriptParam(std::string("could not find a window with given ID"))); return errout;}\
	GetInstance().m_mWindowInfos.find((int)params[0].getFloat())->second.wininfofield=params[1];\
	return std::vector<SGTScriptParam>()

//mouse events
std::vector<SGTScriptParam> SGTGUISystem::Lua_SetOnClickCallback(SGTScript& caller, std::vector<SGTScriptParam> vParams){LUA_CALLBACK_SETTER_MACRO(vParams, parOnClick, caller);}
std::vector<SGTScriptParam> SGTGUISystem::Lua_SetMouseDownCallback(SGTScript& caller, std::vector<SGTScriptParam> vParams){LUA_CALLBACK_SETTER_MACRO(vParams, parMouseDown, caller);}
std::vector<SGTScriptParam> SGTGUISystem::Lua_SetMouseUpCallback(SGTScript& caller, std::vector<SGTScriptParam> vParams){LUA_CALLBACK_SETTER_MACRO(vParams, parMouseUp, caller);}
std::vector<SGTScriptParam> SGTGUISystem::Lua_SetMouseMoveCallback(SGTScript& caller, std::vector<SGTScriptParam> vParams){LUA_CALLBACK_SETTER_MACRO(vParams, parMouseMove, caller);}
std::vector<SGTScriptParam> SGTGUISystem::Lua_SetMouseHoverInCallback(SGTScript& caller, std::vector<SGTScriptParam> vParams){LUA_CALLBACK_SETTER_MACRO(vParams, parMouseHoverIn, caller);}
std::vector<SGTScriptParam> SGTGUISystem::Lua_SetMouseHoverOutCallback(SGTScript& caller, std::vector<SGTScriptParam> vParams){LUA_CALLBACK_SETTER_MACRO(vParams, parMouseHoverOut, caller);}
//keyboard events
std::vector<SGTScriptParam> SGTGUISystem::Lua_SetOnCharCallback(SGTScript& caller, std::vector<SGTScriptParam> vParams){LUA_CALLBACK_SETTER_MACRO(vParams, parOnChar, caller);}
std::vector<SGTScriptParam> SGTGUISystem::Lua_SetKeyDownCallback(SGTScript& caller, std::vector<SGTScriptParam> vParams){LUA_CALLBACK_SETTER_MACRO(vParams, parKeyDown, caller);}
std::vector<SGTScriptParam> SGTGUISystem::Lua_SetKeyUpCallback(SGTScript& caller, std::vector<SGTScriptParam> vParams){LUA_CALLBACK_SETTER_MACRO(vParams, parKeyUp, caller);}