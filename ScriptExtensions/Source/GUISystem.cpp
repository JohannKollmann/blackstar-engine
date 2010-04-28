#include "GUISystem.h"
#include "IceInput.h"
#include "IceMain.h"
#include "IceMessageSystem.h"
#include "IceUtils.h"
#include "IceSceneManager.h"


GUISystem::GUISystem(void)
{

	m_iCursorHandle=-1;
	m_iFocusWin=-1;
	m_fMaxZ=0.0001f;
	m_fZStep=m_fMaxZ/1000.0f;

	Ice::MessageSystem::Instance().JoinNewsgroup(this, "MOUSE_MOVE");
	Ice::MessageSystem::Instance().JoinNewsgroup(this, "MOUSE_DOWN");
	Ice::MessageSystem::Instance().JoinNewsgroup(this, "MOUSE_UP");
	Ice::MessageSystem::Instance().JoinNewsgroup(this, "KEY_DOWN");
	Ogre::SceneNode* pNode=Ice::Main::Instance().GetOgreSceneMgr()->getRootSceneNode()->createChildSceneNode("GUISystemNode");
	pNode->setPosition(0, 0, 0);
	m_fXPos=m_fYPos=0.5f;

	Window w=MakeWindow(0,0,0,0,0,0);
	w.Bake();
	SetVisible(w.GetHandle(), false);
	m_iCursorHandle=w.GetHandle();

	
	Ice::ScriptSystem::GetInstance().ShareCFunction("gui_create_window", Lua_CreateWindow);
	Ice::ScriptSystem::GetInstance().ShareCFunction("gui_create_subwindow", Lua_CreateSubWindow);
	Ice::ScriptSystem::GetInstance().ShareCFunction("gui_bake_window", Lua_BakeWindow);
	Ice::ScriptSystem::GetInstance().ShareCFunction("gui_delete_window", Lua_DeleteWindow);

	Ice::ScriptSystem::GetInstance().ShareCFunction("gui_set_window_material", Lua_SetMaterial);
	Ice::ScriptSystem::GetInstance().ShareCFunction("gui_set_visible", Lua_SetWindowVisible);
	Ice::ScriptSystem::GetInstance().ShareCFunction("gui_set_foreground_window", Lua_SetForegroundWindow);
	Ice::ScriptSystem::GetInstance().ShareCFunction("gui_set_focus", Lua_SetFocus);
	Ice::ScriptSystem::GetInstance().ShareCFunction("gui_move_window", Lua_MoveWindow);
	Ice::ScriptSystem::GetInstance().ShareCFunction("gui_get_window_pos", Lua_GetWindowPos);
	Ice::ScriptSystem::GetInstance().ShareCFunction("gui_set_cursor_pos", Lua_SetCursorPos);
	

	Ice::ScriptSystem::GetInstance().ShareCFunction("gui_get_screen_coords", Lua_GetScreenCoords);
	
	Ice::ScriptSystem::GetInstance().ShareCFunction("gui_get_cursor", Lua_GetCursor);
	Ice::ScriptSystem::GetInstance().ShareCFunction("gui_set_cursor", Lua_SetCursor);

	Ice::ScriptSystem::GetInstance().ShareCFunction("gui_change_font_material", Lua_ChangeFontMaterial);
	Ice::ScriptSystem::GetInstance().ShareCFunction("gui_create_font_material", Lua_CreateFontMaterial);
	Ice::ScriptSystem::GetInstance().ShareCFunction("gui_create_font_texture", Lua_CreateFontTexture);
	Ice::ScriptSystem::GetInstance().ShareCFunction("gui_delete_texture", Lua_DeleteTexture);

	//mouse events
	Ice::ScriptSystem::GetInstance().ShareCFunction("gui_set_on_click_callback", Lua_SetOnClickCallback);
	Ice::ScriptSystem::GetInstance().ShareCFunction("gui_set_mouse_down_callback", Lua_SetMouseDownCallback);
	Ice::ScriptSystem::GetInstance().ShareCFunction("gui_set_mouse_up_callback", Lua_SetMouseUpCallback);
	Ice::ScriptSystem::GetInstance().ShareCFunction("gui_set_mouse_move_callback", Lua_SetMouseMoveCallback);
	Ice::ScriptSystem::GetInstance().ShareCFunction("gui_set_mouse_hover_in_callback", Lua_SetMouseHoverInCallback);
	Ice::ScriptSystem::GetInstance().ShareCFunction("gui_set_mouse_hover_out_callback", Lua_SetMouseHoverOutCallback);
	//keyboard events
	Ice::ScriptSystem::GetInstance().ShareCFunction("gui_set_on_char_callback", Lua_SetOnCharCallback);
	Ice::ScriptSystem::GetInstance().ShareCFunction("gui_set_key_down_callback", Lua_SetKeyDownCallback);
	Ice::ScriptSystem::GetInstance().ShareCFunction("gui_set_key_up_callback", Lua_SetKeyUpCallback);
	//input states
	Ice::ScriptSystem::GetInstance().ShareCFunction("input_get_key_down", Lua_GetKeyDown);
	Ice::ScriptSystem::GetInstance().ShareCFunction("input_get_mouse_buttons", Lua_GetMouseButtons);
}

void
GUISystem::ReceiveMessage(Ice::Msg &msg)
{
	if(msg.type == "KEY_DOWN" && m_iFocusWin!=-1)
	{
		SWindowInfo wininfo=GUISystem::GetInstance().m_mWindowInfos.find(m_iFocusWin)->second;
		while(wininfo.iParentHandle!=-1)
			wininfo=GUISystem::GetInstance().m_mWindowInfos.find(wininfo.iParentHandle)->second;
		if(m_mWindowInfos.find(m_iFocusWin)->second.bVisible && wininfo.bVisible && m_mWindowInfos.find(m_iFocusWin)->second.parKeyDown.getType()==Ice::ScriptParam::PARM_TYPE_FUNCTION)
		{
			std::vector<Ice::ScriptParam> vResults;
			vResults.push_back(Ice::ScriptParam(m_iFocusWin));
			vResults.push_back(Ice::ScriptParam(msg.params.GetInt("KEY_ID_OIS")));
			Ice::ScriptSystem::RunCallbackFunction(m_mWindowInfos.find(m_iFocusWin)->second.parKeyDown, vResults);
		}
	}
	/*std::map<int, SWindowInfo>::const_iterator it=m_mWindowInfos.begin();
	for(; it!=m_mWindowInfos.end(); it++)
		if(it->second.iParentHandle==-1)
			Ice::Main::Instance().GetOgreSceneMgr()->getEntity(it->second.strName)->setVisible(m_bMenuActive);*/

	if (msg.type == "MOUSE_MOVE")
	{
		m_fXPos+=(float)msg.params.GetInt("ROT_X_REL")/(float)Ice::Main::Instance().GetViewport()->getActualWidth();
		m_fYPos+=(float)msg.params.GetInt("ROT_Y_REL")/(float)Ice::Main::Instance().GetViewport()->getActualHeight();
		//MessageBox(0, "", "", 0);
		if(m_fXPos>1.0)
			m_fXPos=1.0;
		if(m_fXPos<0.0)
			m_fXPos=0.0;
		if(m_fYPos>1.0)
			m_fYPos=1.0;
		if(m_fYPos<0.0)
			m_fYPos=0.0;
		//Ice::Main::Instance().GetOgreSceneMgr()->getSceneNode("mySceneNode")->setPosition(m_fXPos, m_fYPos, 0);
		//Ice::Main::Instance().GetOgreSceneMgr()->getEntity("bleh")->getSubEntity(0)->setCustomParameter(0, Ogre::Vector4(m_fXPos, m_fYPos, 0, 0));
		Window(m_iCursorHandle).Move(m_fXPos, m_fYPos);
		//call mousemove on the cursor
		if(m_mWindowInfos.find(m_iCursorHandle)->second.parMouseMove.getType()==Ice::ScriptParam::PARM_TYPE_FUNCTION)
		{
			std::vector<Ice::ScriptParam> parms(1, Ice::ScriptParam(m_iCursorHandle));
			parms.push_back(Ice::ScriptParam(m_fXPos));
			parms.push_back(Ice::ScriptParam(m_fYPos));
			Ice::ScriptSystem::RunCallbackFunction(m_mWindowInfos.find(m_iCursorHandle)->second.parMouseMove, parms);
		}

		std::list<int>::iterator itZ=m_lZOrder.begin();
		std::map<int, SWindowInfo>::iterator it;
		for(; itZ!=m_lZOrder.end(); itZ++)
		{
			it=m_mWindowInfos.find(*itZ);
			if(!it->second.bVisible)
					continue;
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
		if(itZ!=m_lZOrder.end())
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
			//call the mousemove event
			if(it->second.parMouseMove.getType()==Ice::ScriptParam::PARM_TYPE_FUNCTION)
			{
				std::vector<Ice::ScriptParam> parms(1, Ice::ScriptParam(it->first));
				parms.push_back(Ice::ScriptParam(m_fXPos));
				parms.push_back(Ice::ScriptParam(m_fYPos));
				Ice::ScriptSystem::RunCallbackFunction(it->second.parMouseMove, parms);
			}

			if(iCurrHover!=m_iHoverWin)
			{
				if(iCurrHover!=-1)
					if(it->second.parMouseHoverIn.getType()==Ice::ScriptParam::PARM_TYPE_FUNCTION)
					{
						std::vector<Ice::ScriptParam> parms(1, Ice::ScriptParam(it->first));
						Ice::ScriptSystem::RunCallbackFunction(it->second.parMouseHoverIn, parms);
					}
				if(m_iHoverWin!=-1)
					if(m_mWindowInfos.find(m_iHoverWin)->second.parMouseHoverOut.getType()==Ice::ScriptParam::PARM_TYPE_FUNCTION)
					{
						std::vector<Ice::ScriptParam> parms(1, m_iHoverWin);
						Ice::ScriptSystem::RunCallbackFunction(m_mWindowInfos.find(m_iHoverWin)->second.parMouseHoverOut, parms);
					}
			}
		}
		else
		{
			if(m_mWindowInfos.find(m_iHoverWin)!=m_mWindowInfos.end() && iCurrHover!=m_iHoverWin)
				if(m_mWindowInfos.find(m_iHoverWin)->second.parMouseHoverOut.getType()==Ice::ScriptParam::PARM_TYPE_FUNCTION)
				{
					std::vector<Ice::ScriptParam> parms(1, m_iHoverWin);
					Ice::ScriptSystem::RunCallbackFunction(m_mWindowInfos.find(m_iHoverWin)->second.parMouseHoverOut, parms);
				}
		}
		m_iHoverWin=iCurrHover;
	}

	if(msg.type == "MOUSE_DOWN" || msg.type == "MOUSE_UP")
		if(OIS::MB_Left==msg.params.GetInt("MOUSE_ID"))
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
				if(!it->second.bVisible)
					continue;
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

				/*if(it->second.parMouseHoverOut.getType()==Ice::ScriptParam::PARM_TYPE_FUNCTION)
				{
					std::vector<Ice::ScriptParam> parms(1, m_iHoverWin);
					Ice::ScriptSystem::RunCallbackFunction(it->second.parMouseHoverOut, parms);
				}*/

				std::vector<Ice::ScriptParam> parms(1, Ice::ScriptParam(it->first));
				parms.push_back(Ice::ScriptParam(m_fXPos));
				parms.push_back(Ice::ScriptParam(m_fYPos));
				if(it->second.parMouseDown.getType()==Ice::ScriptParam::PARM_TYPE_FUNCTION && msg.type=="MOUSE_DOWN")
					Ice::ScriptSystem::RunCallbackFunction(it->second.parMouseDown, parms);
				if(it->second.parMouseUp.getType()==Ice::ScriptParam::PARM_TYPE_FUNCTION && msg.type=="MOUSE_UP")
					Ice::ScriptSystem::RunCallbackFunction(it->second.parMouseUp, parms);

			}
			//send it to the cursor itself, too
			it=m_mWindowInfos.find(m_iCursorHandle);
			std::vector<Ice::ScriptParam> parms(1, m_iCursorHandle);
			parms.push_back(Ice::ScriptParam(m_fXPos));
			parms.push_back(Ice::ScriptParam(m_fYPos));
			if(it!=m_mWindowInfos.end())
			{
				if(it->second.parMouseDown.getType()==Ice::ScriptParam::PARM_TYPE_FUNCTION && msg.type=="MOUSE_DOWN")
					Ice::ScriptSystem::RunCallbackFunction(it->second.parMouseDown, parms);
				if(it->second.parMouseUp.getType()==Ice::ScriptParam::PARM_TYPE_FUNCTION && msg.type=="MOUSE_UP")
					Ice::ScriptSystem::RunCallbackFunction(it->second.parMouseUp, parms);
			}
		}	
}

GUISystem&
GUISystem::GetInstance()
{
	static GUISystem singleton;
	return singleton;
}

GUISystem::Window
GUISystem::MakeWindow(float x, float y, float w, float h, float fUScale, float fVScale)
{
	int iHandle=Ice::SceneManager::Instance().RequestID();
	Window win(iHandle);
	SWindowInfo wininfo;
	wininfo.iParentHandle=-1;
	wininfo.strName=Ogre::String("GUI-System-Window ") + Ice::SceneManager::Instance().RequestIDStr();
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
GUISystem::Window::Bake()
{
	GUISystem::SWindowInfo wininfo=GetInstance().m_mWindowInfos.find(m_iHandle)->second;
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
		GUISystem::SWindowInfo subwininfo=GUISystem::GetInstance().m_mWindowInfos.find(vSubWindows[iSubWin])->second;
		//get z level of this sub-window
		int iLevel=0;
		GUISystem::SWindowInfo tempinfo=subwininfo;
		while(tempinfo.iParentHandle!=-1)
		{
			tempinfo=GUISystem::GetInstance().m_mWindowInfos.find(tempinfo.iParentHandle)->second;
			iLevel++;
		}
		if(iLevel>wininfo.iDepth)
			wininfo.iDepth=iLevel;
		//while(tempinfo
		float z=-GUISystem::GetInstance().m_fZStep*(float)iLevel;
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
		SWindowInfo subwininfo=GUISystem::GetInstance().m_mWindowInfos.find(vSubWindows[iSubWin])->second;
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
	Ogre::Entity *ent = Ice::Main::Instance().GetOgreSceneMgr()->createEntity(wininfo.strName, wininfo.strName);
	ent->setCastShadows(false);
	ent->setRenderQueueGroup(Ogre::RENDER_QUEUE_OVERLAY);
	Ice::Main::Instance().GetOgreSceneMgr()->getSceneNode("GUISystemNode")->attachObject(ent);

	GetInstance().m_mWindowInfos.find(m_iHandle)->second.bWasBaked=true;

	Ice::Main::Instance().GetOgreSceneMgr()->getEntity(wininfo.strName)->getSubEntity(0)->setCustomParameter(0, Ogre::Vector4(x, y, 0, 0));
	if(wininfo.strMaterial.size())
		Ice::Main::Instance().GetOgreSceneMgr()->getEntity(wininfo.strName)->getSubEntity(0)->setMaterialName(wininfo.strMaterial);
	for(iSubWin=0; iSubWin<nSubWindows; iSubWin++)
	{
		SWindowInfo subwininfo=GetInstance().m_mWindowInfos.find(vSubWindows[iSubWin])->second;
		Ogre::SubEntity* pSubEnt=Ice::Main::Instance().GetOgreSceneMgr()->getEntity(wininfo.strName)->getSubEntity(subwininfo.strName);
		pSubEnt->setCustomParameter(0, Ogre::Vector4(x, y, 0, 0));
		if(wininfo.strMaterial.size())
			pSubEnt->setMaterialName(subwininfo.strMaterial);
	}
	GUISystem::GetInstance().SetForegroundWindow(m_iHandle);
	/*GUISystem::GetInstance().SetFocus(m_iHandle);*/
}

std::list<int>
GUISystem::Window::FindSubWindows(int iHandle)
{
	std::vector<int> vSubWins=GUISystem::GetInstance().m_mWindowInfos.find(iHandle)->second.vSubWindows;
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
GUISystem::Window::GetSubWindows()
{
	std::vector<int> vRes;
	std::list<int> lSubWins=FindSubWindows(m_iHandle);
	for(std::list<int>::iterator it=lSubWins.begin(); it!=lSubWins.end(); it++)
		vRes.push_back(*it);
	return vRes;
}


void
GUISystem::Window::SetMaterial(Ogre::String strMat)
{
	std::map<int, SWindowInfo>::iterator it=GetInstance().m_mWindowInfos.find(m_iHandle);
	it->second.strMaterial=strMat;
	if(it->second.bWasBaked)
		Ice::Main::Instance().GetOgreSceneMgr()->getEntity(it->second.strName)->getSubEntity("windowface")->setMaterialName(strMat);
}

void
GUISystem::SubWindow::SetMaterial(Ogre::String strMat)
{
	GetInstance().m_mWindowInfos.find(m_iHandle)->second.strMaterial=strMat;
	GUISystem::SWindowInfo wininfo=GetInstance().m_mWindowInfos.find(m_iHandle)->second;
	Ogre::String strSubMeshName=wininfo.strName;
	while(wininfo.iParentHandle!=-1)
		wininfo=GUISystem::GetInstance().m_mWindowInfos.find(wininfo.iParentHandle)->second;
	//Ogre::Mesh* pMesh=Ogre::MeshManager::getSingleton().getByName(wininfo.strName);
	///pMesh->getSubMesh(strSubMeshName)->setMaterialName(strMat);
	if(wininfo.bWasBaked)
		Ice::Main::Instance().GetOgreSceneMgr()->getEntity(wininfo.strName)->getSubEntity(strSubMeshName)->setMaterialName(strMat);//getMesh()->getSubMesh(strSubMeshName)->setMaterialName(strMat);
}


GUISystem::SubWindow
GUISystem::CreateSubWindow(int iParentHandle, float x, float y, float w, float h, float fUScale, float fVScale)
{
	int iHandle=Ice::SceneManager::Instance().RequestID();
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
	wininfo.strName=Ogre::String("GUI-System-Subwindow ") + Ice::SceneManager::Instance().RequestIDStr();
	m_mWindowInfos.insert(std::make_pair<int, SWindowInfo>(iHandle, wininfo));
	return SubWindow(iHandle);
}

GUISystem::Window::Window(int iHandle){m_iHandle=iHandle;}
GUISystem::Window::Window(){m_iHandle=-1;}
GUISystem::SubWindow::SubWindow(int iHandle){m_iHandle=iHandle;}

void
GUISystem::Window::Move(float x, float y)
{
	GetInstance().m_mWindowInfos.find(m_iHandle)->second.x=x;
	GetInstance().m_mWindowInfos.find(m_iHandle)->second.y=y;
	SWindowInfo wi=GetInstance().m_mWindowInfos.find(m_iHandle)->second;
	Ogre::Entity* pEnt=Ice::Main::Instance().GetOgreSceneMgr()->getEntity(wi.strName);
	for(unsigned int iSubEnt=0; iSubEnt<pEnt->getNumSubEntities(); iSubEnt++)
	{
		Ogre::Vector4 vCustom=pEnt->getSubEntity(iSubEnt)->getCustomParameter(0);
		vCustom.x=x;
		vCustom.y=y;
		pEnt->getSubEntity(iSubEnt)->setCustomParameter(0, vCustom);
	}
}

int
GUISystem::Window::GetHandle()
{
	return m_iHandle;
}

int
GUISystem::SubWindow::GetHandle()
{
	return m_iHandle;
}

void
GUISystem::SetForegroundWindow(int iHandle)
{
	//get window for subwindow
	SWindowInfo wininfo=m_mWindowInfos.find(iHandle)->second;
	while(wininfo.iParentHandle!=-1)
	{
		iHandle=wininfo.iParentHandle;
		wininfo=GUISystem::GetInstance().m_mWindowInfos.find(wininfo.iParentHandle)->second;
	}

	if(m_iCursorHandle!=FindParentWindow(iHandle) && m_lZOrder.size())
	{
		m_lZOrder.remove(iHandle);
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
		Ogre::Entity* pEnt=Ice::Main::Instance().GetOgreSceneMgr()->getEntity(wininfo.strName);
		for(unsigned int iSubEnt=0; iSubEnt<pEnt->getNumSubEntities(); iSubEnt++)
		{
			Ogre::Vector4 vCustom=pEnt->getSubEntity(iSubEnt)->getCustomParameter(0);
			vCustom.z=(float)iCurrDepth*m_fZStep;
			pEnt->getSubEntity(iSubEnt)->setCustomParameter(0, vCustom);
		}
	}
}

void
GUISystem::SetFocus(int iHandle)
{
	m_iFocusWin=iHandle;
}

int
GUISystem::FindParentWindow(int iSubWindowHandle)
{
	GUISystem::SWindowInfo subwininfo=m_mWindowInfos.find(iSubWindowHandle)->second;
	int iHandle=iSubWindowHandle;
	while(subwininfo.iParentHandle!=-1)
	{
		iHandle=subwininfo.iParentHandle;
		subwininfo=m_mWindowInfos.find(subwininfo.iParentHandle)->second;
	}
	return iHandle;
}


void
GUISystem::SetVisible(int iHandle, bool bVisible)
{
	SWindowInfo subwininfo=m_mWindowInfos.find(iHandle)->second;
	m_mWindowInfos.find(iHandle)->second.bVisible=bVisible;
	if(subwininfo.iParentHandle==-1)
	{//hide a window, thats quite easy
		Ice::Main::Instance().GetOgreSceneMgr()->getEntity(subwininfo.strName)->setVisible(bVisible);
		return;
	}
	//hiding subwindows requires more work
	SWindowInfo wininfo=m_mWindowInfos.find(FindParentWindow(iHandle))->second;
	Ogre::Vector4 vCustom=Ice::Main::Instance().GetOgreSceneMgr()->getEntity(wininfo.strName)->getSubEntity(subwininfo.strName)->getCustomParameter(0);
	vCustom.x=bVisible ? wininfo.x : 10;
	Ice::Main::Instance().GetOgreSceneMgr()->getEntity(wininfo.strName)->getSubEntity(subwininfo.strName)->setCustomParameter(0, vCustom);
}

bool
GUISystem::GetVisible(int iHandle)
{
	return m_mWindowInfos.find(iHandle)->second.bVisible;
}

void
GUISystem::SetCursor(int iHandle)
{
	if(m_mWindowInfos.find(iHandle)->second.iParentHandle!=-1)
		return;
	m_iCursorHandle=iHandle;
	m_lZOrder.remove(iHandle);
	m_lZOrder.push_front(iHandle);
	SetForegroundWindow(iHandle);
}

void
GUISystem::DeleteWindow(int iHandle)
{
	std::map<int, GUISystem::SWindowInfo>::iterator it;
	if((it=m_mWindowInfos.find(iHandle))!=m_mWindowInfos.end())
	{//if valid handle..
		if(it->second.bWasBaked)
		{
			m_lZOrder.remove(iHandle);
			SetForegroundWindow(m_lZOrder.front());
			if(m_iFocusWin==iHandle)
				GUISystem::GetInstance().SetFocus(m_lZOrder.front());
			if(m_iHoverWin==iHandle)
				m_iHoverWin=-1;
			Ice::Main::Instance().GetOgreSceneMgr()->getSceneNode("GUISystemNode")->detachObject(Ice::Main::Instance().GetOgreSceneMgr()->getEntity(it->second.strName));
			Ogre::MeshManager::getSingleton().remove(it->second.strName);
		}
		//delete subwindows
		std::list<int> lSubWindows=Window::FindSubWindows(iHandle);
		for(std::list<int>::const_iterator itl=lSubWindows.begin(); itl!=lSubWindows.end(); itl++)
		{
			if(m_iFocusWin==*itl)
				GUISystem::GetInstance().SetFocus(m_lZOrder.front());
			if(m_iHoverWin==*itl)
				m_iHoverWin=-1;
			m_mWindowInfos.erase(m_mWindowInfos.find(*itl));
		}
		m_mWindowInfos.erase(it);
	}
}

void
GUISystem::Clear()
{
	for(std::map<int, SWindowInfo>::iterator it=m_mWindowInfos.begin(); it!=m_mWindowInfos.end(); it++)
	{
		int iHandle=FindParentWindow(it->first);
		SWindowInfo wininfo=m_mWindowInfos.find(iHandle)->second;
		if(wininfo.bWasBaked)
		{
			Ice::Main::Instance().GetOgreSceneMgr()->getSceneNode("GUISystemNode")->detachObject(wininfo.strName);
			Ogre::MeshManager::getSingleton().unload(wininfo.strName);
			Ogre::MeshManager::getSingleton().remove(wininfo.strName);
			m_mWindowInfos.find(iHandle)->second.bWasBaked=false;
		}
	}
	//delete materials created by font textures
	for(std::list<Ogre::String>::iterator it=m_lMaterials.begin(); it!=m_lMaterials.end(); it++)
		if(Ogre::MaterialManager::getSingleton().resourceExists(*it))
		{
			Ogre::MaterialManager::getSingleton().unload(*it);
			Ogre::MaterialManager::getSingleton().remove(*it);
		}
	//delete textures created by font textures
	for(std::list<Ogre::String>::iterator it=m_lTextures.begin(); it!=m_lTextures.end(); it++)
		if(Ogre::TextureManager::getSingleton().resourceExists(*it))
		{
			Ogre::TextureManager::getSingleton().unload(*it);
			Ogre::TextureManager::getSingleton().remove(*it);
		}
	m_mFontTextures.clear();
	m_lZOrder.clear();
	m_mWindowInfos.clear();
	m_lMaterials.clear();
	m_lTextures.clear();

	m_iCursorHandle=-1;
	m_iFocusWin=-1;
	m_fXPos=m_fYPos=0.5f;

	Window w=MakeWindow(0,0,0,0,0,0);
	w.Bake();
	SetVisible(w.GetHandle(), false);
	m_iCursorHandle=w.GetHandle();
}

std::vector<Ice::ScriptParam>
GUISystem::Lua_CreateFontTexture(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams)
{//arguments: spacing file, text, maxwidth, maxheight
//outputs: texture name, actual width, actual height
	std::map<std::string, FontTextures>::iterator it=GetInstance().m_mFontTextures.find(vParams[0].getString());
	if(it==GetInstance().m_mFontTextures.end())
	{//we don't have this spacing in memory, load it
		GetInstance().m_mFontTextures.insert(std::pair<std::string, FontTextures>(vParams[0].getString(), FontTextures(vParams[0].getString())));
		it=GetInstance().m_mFontTextures.find(vParams[0].getString());
	}
	//use that font textures object to crate the texture
	int iActualWidth, iActualHeight;
	std::vector<Ice::ScriptParam> vRes;
	Ogre::TexturePtr pTex=it->second.CreateTextTexture(vParams[1].getString(), (int)vParams[2].getFloat(), (int)vParams[3].getFloat(), iActualWidth, iActualHeight);
	GetInstance().m_lTextures.push_back(pTex->getName());
	vRes.push_back(Ice::ScriptParam(pTex->getName()));
	vRes.push_back(Ice::ScriptParam(iActualWidth));
	vRes.push_back(Ice::ScriptParam(iActualHeight));
	return vRes;
}

std::vector<Ice::ScriptParam>
GUISystem::Lua_CreateFontMaterial(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams)
{//arguments: spacing file, texture name, base material name, pass name, target texture name, maxwidth, maxheight
//outputs: material name
	std::map<std::string, FontTextures>::iterator it=GetInstance().m_mFontTextures.find(vParams[0].getString());
	if(it==GetInstance().m_mFontTextures.end())
	{//we don't have this spacing in memory, load it
		GetInstance().m_mFontTextures.insert(std::pair<std::string, FontTextures>(vParams[0].getString(), FontTextures(vParams[0].getString())));
		it=GetInstance().m_mFontTextures.find(vParams[0].getString());
	}
	Ogre::MaterialPtr pMat=it->second.CreateTextMaterial(Ogre::TextureManager::getSingleton().getByName(vParams[1].getString()), vParams[2].getString(), vParams[3].getString(), vParams[4].getString(), (int)vParams[5].getFloat(), (int)vParams[6].getFloat());
	GetInstance().m_lMaterials.push_back(pMat->getName());
	return std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(pMat->getName()));
}

std::vector<Ice::ScriptParam>
GUISystem::Lua_ChangeFontMaterial(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams)
{//arguments: spacing file, material name, texture name, pass name, target texture name, maxwidth, maxheight
//outputs: none
	std::map<std::string, FontTextures>::iterator it=GetInstance().m_mFontTextures.find(vParams[0].getString());
	if(it==GetInstance().m_mFontTextures.end())
	{//we don't have this spacing in memory, load it
		GetInstance().m_mFontTextures.insert(std::pair<std::string, FontTextures>(vParams[0].getString(), FontTextures(vParams[0].getString())));
		it=GetInstance().m_mFontTextures.find(vParams[0].getString());
	}
	it->second.ChangeTextMaterial(
		Ogre::MaterialManager::getSingleton().getByName(vParams[1].getString()),
		Ogre::TextureManager::getSingleton().getByName(vParams[2].getString()),
		vParams[3].getString(),
		vParams[4].getString(),
		(int)vParams[5].getFloat(),
		(int)vParams[6].getFloat());

	return std::vector<Ice::ScriptParam>();
}

std::vector<Ice::ScriptParam>
GUISystem::Lua_DeleteTexture(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams)
{//arguments: texture name
	Ogre::TextureManager::getSingleton().remove(vParams[0].getString());
	return std::vector<Ice::ScriptParam>();
}

std::vector<Ice::ScriptParam>
GUISystem::Lua_DeleteMaterial(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams)
{//arguments: texture name
	Ogre::ResourcePtr p;
	if(!(p=Ogre::MaterialManager::getSingleton().getByName(vParams[0].getString())).isNull())
		Ogre::MaterialManager::getSingleton().remove(p);
	return std::vector<Ice::ScriptParam>();
}

std::vector<Ice::ScriptParam>
GUISystem::Lua_CreateWindow(Ice::Script &caller, std::vector<Ice::ScriptParam> vParams)
{//input: x,y,w,h
//return: window-id
	std::vector<Ice::ScriptParam> ret;
	if(vParams.size()!=4)
		return ret;
	if(vParams[0].getType()!=Ice::ScriptParam::PARM_TYPE_FLOAT ||
		vParams[1].getType()!=Ice::ScriptParam::PARM_TYPE_FLOAT ||
		vParams[2].getType()!=Ice::ScriptParam::PARM_TYPE_FLOAT ||
		vParams[3].getType()!=Ice::ScriptParam::PARM_TYPE_FLOAT)
		return ret;
	ret.push_back(GUISystem::GetInstance().MakeWindow(vParams[0].getFloat(), vParams[1].getFloat(), vParams[2].getFloat(), vParams[3].getFloat()).GetHandle());
	return ret;
}

std::vector<Ice::ScriptParam>
GUISystem::Lua_CreateSubWindow(Ice::Script &caller, std::vector<Ice::ScriptParam> vParams)
{//input: x,y,w,h,parent
//return: window-id
	std::vector<Ice::ScriptParam> ret;
	for(int iParm=0; iParm<5; iParm++)
		ret.push_back(Ice::ScriptParam(0.0));
	std::string strError=Ice::Utils::TestParameters(vParams, ret, true);
	ret.clear();
	ret.push_back(Ice::ScriptParam());
	if(strError.size())
	{
		ret.push_back(Ice::ScriptParam(strError));
		return ret;
	}
	if(vParams.size()!=5 && vParams.size()!=7)
	{
		ret.push_back(Ice::ScriptParam(std::string("function expects 5 or 7 parameters")));
		return ret;
	}
	if(vParams.size()==7)
		if(vParams[5].getType()!=Ice::ScriptParam::PARM_TYPE_FLOAT ||
			vParams[6].getType()!=Ice::ScriptParam::PARM_TYPE_FLOAT)
		{
			ret.push_back(Ice::ScriptParam(std::string("parameters 5 and 6 have to be floats")));
			return ret;
		}
	
	ret.clear();
	if(vParams.size()==5)
		ret.push_back(GetInstance().CreateSubWindow((int)vParams[0].getFloat(), vParams[1].getFloat(), vParams[2].getFloat(), vParams[3].getFloat(), vParams[4].getFloat()).GetHandle());
	else
		ret.push_back(GetInstance().CreateSubWindow((int)vParams[0].getFloat(), vParams[1].getFloat(), vParams[2].getFloat(), vParams[3].getFloat(), vParams[4].getFloat(), vParams[5].getFloat(), vParams[6].getFloat()).GetHandle());
	return ret;
}

std::vector<Ice::ScriptParam>
GUISystem::Lua_BakeWindow(Ice::Script &caller, std::vector<Ice::ScriptParam> vParams)
{
	std::vector<Ice::ScriptParam> ret;
	if(vParams.size()!=1)
		return ret;
	if(vParams[0].getType()!=Ice::ScriptParam::PARM_TYPE_FLOAT)
		return ret;
	Window((int)vParams[0].getFloat()).Bake();
	return ret;
}

std::vector<Ice::ScriptParam>
GUISystem::Lua_DeleteWindow(Ice::Script &caller, std::vector<Ice::ScriptParam> vParams)
{
	std::vector<Ice::ScriptParam> errout;
	errout.push_back(Ice::ScriptParam());
	std::string strErrString=Ice::Utils::TestParameters(vParams, std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(0.1)), false);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	GetInstance().DeleteWindow((int)vParams[0].getFloat());
	return std::vector<Ice::ScriptParam>();
}



std::vector<Ice::ScriptParam>
GUISystem::Lua_SetMaterial(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams)
{
	std::vector<Ice::ScriptParam> ret;
	if(vParams.size()!=2)
		return ret;
	if(vParams[0].getType()!=Ice::ScriptParam::PARM_TYPE_FLOAT || vParams[1].getType()!=Ice::ScriptParam::PARM_TYPE_STRING)
		return ret;
	SWindowInfo wininfo=GUISystem::GetInstance().m_mWindowInfos.find((int)vParams[0].getFloat())->second;
	if(wininfo.iParentHandle==-1)//is top-level-window
		Window((int)vParams[0].getFloat()).SetMaterial(vParams[1].getString());
	else
		SubWindow((int)vParams[0].getFloat()).SetMaterial(vParams[1].getString());

	return ret;
}

std::vector<Ice::ScriptParam>
GUISystem::Lua_SetWindowVisible(Ice::Script &caller, std::vector<Ice::ScriptParam> vParams)
{//inputs: window-id,bool
	std::vector<Ice::ScriptParam> ret;
	if(vParams.size()!=2)
		return ret;
	if(vParams[0].getType()!=Ice::ScriptParam::PARM_TYPE_FLOAT ||
		vParams[1].getType()!=Ice::ScriptParam::PARM_TYPE_BOOL)
		return ret;
	GUISystem::GetInstance().SetVisible((int)vParams[0].getFloat(), vParams[1].getBool());
	return ret;
}
std::vector<Ice::ScriptParam>
GUISystem::Lua_SetForegroundWindow(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams)
{
	std::vector<Ice::ScriptParam> errout;
	errout.push_back(Ice::ScriptParam());
	std::string strErrString=Ice::Utils::TestParameters(vParams, std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(0.1)), false);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	if(GetInstance().m_mWindowInfos.find((int)vParams[0].getFloat())==GetInstance().m_mWindowInfos.end())
	{
		errout.push_back(Ice::ScriptParam(std::string("could not find a window with given ID")));
		return errout;
	}
	GetInstance().SetForegroundWindow((int)vParams[0].getFloat());
	return std::vector<Ice::ScriptParam>();
}

std::vector<Ice::ScriptParam>
GUISystem::Lua_SetFocus(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams)
{
	std::vector<Ice::ScriptParam> errout;
	errout.push_back(Ice::ScriptParam());
	std::string strErrString=Ice::Utils::TestParameters(vParams, std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(0.1)), false);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	if(GetInstance().m_mWindowInfos.find((int)vParams[0].getFloat())==GetInstance().m_mWindowInfos.end())
	{
		errout.push_back(Ice::ScriptParam(std::string("could not find a window with given ID")));
		return errout;
	}
	GetInstance().SetFocus((int)vParams[0].getFloat());
	return std::vector<Ice::ScriptParam>();
}

std::vector<Ice::ScriptParam>
GUISystem::Lua_MoveWindow(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams)
{
	std::vector<Ice::ScriptParam> errout;
	errout.push_back(Ice::ScriptParam());
	std::vector<Ice::ScriptParam> vRef=std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(0.1));
	vRef.push_back(Ice::ScriptParam(0.1));
	vRef.push_back(Ice::ScriptParam(0.1));

	std::string strErrString=Ice::Utils::TestParameters(vParams, vRef, false);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	
	std::map<int, SWindowInfo>::const_iterator it=GetInstance().m_mWindowInfos.find((int)vParams[0].getFloat());
	if(it==GetInstance().m_mWindowInfos.end())
	{
		errout.push_back(Ice::ScriptParam(std::string("could not find a window with given ID")));
		return errout;
	}
	Window(GetInstance().FindParentWindow(it->first)).Move(vParams[1].getFloat(), vParams[2].getFloat());
	return std::vector<Ice::ScriptParam>();
}

std::vector<Ice::ScriptParam>
GUISystem::Lua_GetWindowPos(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams)
{
	std::vector<Ice::ScriptParam> errout;
	errout.push_back(Ice::ScriptParam());
	std::vector<Ice::ScriptParam> vRef=std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(0.1));

	std::string strErrString=Ice::Utils::TestParameters(vParams, vRef, false);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	std::map<int, SWindowInfo>::const_iterator it=GetInstance().m_mWindowInfos.find((int)vParams[0].getFloat());
	if(it==GetInstance().m_mWindowInfos.end())
	{
		errout.push_back(Ice::ScriptParam(std::string("could not find a window with given ID")));
		return errout;
	}
	GUISystem::SWindowInfo wininfo=it->second;
	while(wininfo.iParentHandle!=-1)
		wininfo=GUISystem::GetInstance().m_mWindowInfos.find(wininfo.iParentHandle)->second;

	errout.clear();
	errout.push_back(Ice::ScriptParam(wininfo.x));
	errout.push_back(Ice::ScriptParam(wininfo.y));
	return errout;
}

std::vector<Ice::ScriptParam>
GUISystem::Lua_SetCursorPos(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams)
{
	std::vector<Ice::ScriptParam> errout;
	errout.push_back(Ice::ScriptParam());
	std::vector<Ice::ScriptParam> vRef=std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(0.1));
	vRef.push_back(Ice::ScriptParam(0.1));
	std::string strErrString=Ice::Utils::TestParameters(vParams, vRef, false);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	GetInstance().m_fXPos=vParams[0].getFloat();
	GetInstance().m_fYPos=vParams[1].getFloat();
	return std::vector<Ice::ScriptParam>();
}
std::vector<Ice::ScriptParam>
GUISystem::Lua_GetScreenCoords(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams)
{
	std::vector<Ice::ScriptParam> errout;
	errout.push_back(Ice::ScriptParam());
	std::string strErrString=Ice::Utils::TestParameters(vParams, std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(0.1)), false);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	if(GetInstance().m_mWindowInfos.find((int)vParams[0].getFloat())==GetInstance().m_mWindowInfos.end())
	{
		errout.push_back(Ice::ScriptParam(std::string("could not find a window with given ID")));
		return errout;
	}
	SWindowInfo wi=GetInstance().m_mWindowInfos.find((int)vParams[0].getFloat())->second;
	errout.clear();
	errout.push_back(Ice::ScriptParam(wi.x));
	errout.push_back(Ice::ScriptParam(wi.y));
	errout.push_back(Ice::ScriptParam(wi.w));
	errout.push_back(Ice::ScriptParam(wi.h));
	return errout;
}

std::vector<Ice::ScriptParam>
GUISystem::Lua_GetCursor(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams)
{
	return std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(GetInstance().m_iCursorHandle));
}

std::vector<Ice::ScriptParam> GUISystem::Lua_SetCursor(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams)
{
	std::vector<Ice::ScriptParam> errout;
	errout.push_back(Ice::ScriptParam());
	std::string strErrString=Ice::Utils::TestParameters(vParams, std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(0.1)), false);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	if(GetInstance().m_mWindowInfos.find((int)vParams[0].getFloat())==GetInstance().m_mWindowInfos.end())
	{
		errout.push_back(Ice::ScriptParam(std::string("could not find a window with given ID")));
		return errout;
	}
	GetInstance().SetCursor((int)vParams[0].getFloat());
	return std::vector<Ice::ScriptParam>();
}



//a little macro for the callback-setters. it will test the parameters and generate meaningful error texts
//then set the given field in the window infos
#define LUA_CALLBACK_SETTER_MACRO(params, wininfofield, script)\
	std::vector<Ice::ScriptParam> errout(1, Ice::ScriptParam());\
	std::vector<Ice::ScriptParam> vRef(1, Ice::ScriptParam(0.1));\
	vRef.push_back(Ice::ScriptParam("", script));\
	std::string strErrString=Ice::Utils::TestParameters(vParams, vRef, false);\
	if(strErrString.length())\
		{errout.push_back(strErrString);return errout;}\
	if(GetInstance().m_mWindowInfos.find((int)params[0].getFloat())==GetInstance().m_mWindowInfos.end())\
		{errout.push_back(Ice::ScriptParam(std::string("could not find a window with given ID"))); return errout;}\
	GetInstance().m_mWindowInfos.find((int)params[0].getFloat())->second.wininfofield=params[1];\
	return std::vector<Ice::ScriptParam>();

//mouse events
std::vector<Ice::ScriptParam> GUISystem::Lua_SetOnClickCallback(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams){LUA_CALLBACK_SETTER_MACRO(vParams, parOnClick, caller);}
std::vector<Ice::ScriptParam> GUISystem::Lua_SetMouseDownCallback(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams){LUA_CALLBACK_SETTER_MACRO(vParams, parMouseDown, caller);}
std::vector<Ice::ScriptParam> GUISystem::Lua_SetMouseUpCallback(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams){LUA_CALLBACK_SETTER_MACRO(vParams, parMouseUp, caller);}
std::vector<Ice::ScriptParam> GUISystem::Lua_SetMouseMoveCallback(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams){LUA_CALLBACK_SETTER_MACRO(vParams, parMouseMove, caller);}
std::vector<Ice::ScriptParam> GUISystem::Lua_SetMouseHoverInCallback(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams){LUA_CALLBACK_SETTER_MACRO(vParams, parMouseHoverIn, caller);}
std::vector<Ice::ScriptParam> GUISystem::Lua_SetMouseHoverOutCallback(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams){LUA_CALLBACK_SETTER_MACRO(vParams, parMouseHoverOut, caller);}
//keyboard events
std::vector<Ice::ScriptParam> GUISystem::Lua_SetOnCharCallback(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams){LUA_CALLBACK_SETTER_MACRO(vParams, parOnChar, caller);}
std::vector<Ice::ScriptParam> GUISystem::Lua_SetKeyDownCallback(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams){LUA_CALLBACK_SETTER_MACRO(vParams, parKeyDown, caller);}
std::vector<Ice::ScriptParam> GUISystem::Lua_SetKeyUpCallback(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams){LUA_CALLBACK_SETTER_MACRO(vParams, parKeyUp, caller);}

std::vector<Ice::ScriptParam>
GUISystem::Lua_GetKeyDown(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams)
{
	std::vector<Ice::ScriptParam> errout;
	errout.push_back(Ice::ScriptParam());
	std::string strErrString=Ice::Utils::TestParameters(vParams, std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(0.1)), false);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	return std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(Ice::Main::Instance().GetInputManager()->isKeyDown((OIS::KeyCode)(int)vParams[0].getFloat())));
}

std::vector<Ice::ScriptParam>
GUISystem::Lua_GetMouseButtons(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams)
{
	std::vector<Ice::ScriptParam> errout;
	errout.push_back(Ice::ScriptParam());
	std::string strErrString=Ice::Utils::TestParameters(vParams, std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(0.1)), false);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	errout.clear();
	Ice::EMouseButtons buttons=Ice::Main::Instance().GetInputManager()->getMouseButton();
	errout.push_back(Ice::ScriptParam((bool)(buttons==Ice::MOUSE_LEFT || buttons==Ice::MOUSE_BOTH)));
	errout.push_back(Ice::ScriptParam((bool)(buttons==Ice::MOUSE_RIGHT || buttons==Ice::MOUSE_BOTH)));
	return errout;
}