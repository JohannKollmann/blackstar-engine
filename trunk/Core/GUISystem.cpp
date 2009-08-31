#include "GUISystem.h"
#include <windows.h>
#include "SGTInput.h"
#include "SGTScriptableInstances.h"


const float SGTGUISystem::m_fFactor=0.001f;
const float SGTGUISystem::cfFrameWidth=0.02f;

SGTGUISystem::SGTGUISystem(void)
{
	SGTScriptableInstances::GetInstance();
	m_iCursorHandle=-1;
	m_fMaxZ=0.0001f;
	m_fZStep=m_fMaxZ/1000.0f;

	SGTMessageSystem::Instance().JoinNewsgroup(this, "MOUSE_MOVE");
	SGTMessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
	SGTMessageSystem::Instance().JoinNewsgroup(this, "MOUSE_DOWN");
	SGTMessageSystem::Instance().JoinNewsgroup(this, "KEY_DOWN");
	Ogre::SceneNode* pNode=SGTMain::Instance().GetOgreSceneMgr()->getRootSceneNode()->createChildSceneNode("SGTGuiSystemNode");
	pNode->setPosition(0, 0, 0);
	m_fXPos=m_fYPos=0.5f;
	Window wMouse=MakeWindow(m_fXPos, m_fYPos, 0.05, 0.05);
	wMouse.Bake();
	wMouse.SetMaterial("gui/mouse");
	SetCursor(wMouse.GetHandle());
	
	Window wExitMenu=MakeWindow(0.25, 0.25, 0.5, 0.5);
/*	SGTGUISystem::SubWindow wContinue=CreateSubWindow(0.25, 0.12, 0.5, 0.20, wExitMenu.GetHandle());
	SGTGUISystem::SubWindow wStart=CreateSubWindow(0.25, 0.37, 0.5, 0.20, wExitMenu.GetHandle());
	SGTGUISystem::SubWindow wQuit=CreateSubWindow(0.25, 0.63, 0.5, 0.20, wExitMenu.GetHandle());
*/
	SGTGUISystem::SubWindow wClear=CreateSubWindow(0.5, 0.8, 0.49, 0.19, wExitMenu.GetHandle());
	wExitMenu.Bake();
	
	wExitMenu.SetKeyCallback("KeyDownCallback");
	wClear.SetOnClickCallback("ClickCallback");
	wClear.SetHoverInCallback("HoverInCallback");
	wClear.SetHoverOutCallback("HoverOutCallback");
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
	SGTScriptSystem::GetInstance().ShareCFunction("set_window_material", Lua_SetMaterial);
	SGTScriptSystem::GetInstance().ShareCFunction("gui_change_font_material", Lua_ChangeFontMaterial);
	SGTScriptSystem::GetInstance().ShareCFunction("gui_create_font_material", Lua_CreateFontMaterial);
	SGTScriptSystem::GetInstance().ShareCFunction("gui_create_font_texture", Lua_CreateFontTexture);
	SGTScriptSystem::GetInstance().ShareCFunction("gui_delete_texture", Lua_DeleteTexture);
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
				if(m_mWindowInfos.find(m_iFocusWin)->second.strKeyPressCallback.size() && m_bMenuActive)
				{
					int aaOISCodeToAscii[256]=
					{0,0,'1','2','3','4','5','6','7','8','9','0',0,0,'\b',0,
					'Q','W','E','R','T','Z','U','I','O','P','Ü',0,'\n','$','A','S',
					'D','F','G','H','J','K','L','Ö','Ä',0,0,0,'Y','X','C','V',
					'B','N','M',0,0,0,0,0,0,' ',0,0,0,0,0,0,
					0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
					0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
					0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
					0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

					0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
					0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
					0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
					0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
					0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
					0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
					0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
					0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
					};
					if(aaOISCodeToAscii[msg.mData.GetInt("KEY_ID_OIS")])
					{
						char ac[2]={aaOISCodeToAscii[msg.mData.GetInt("KEY_ID_OIS")], '\0'};
						std::vector<SGTScriptParam> vResults;
						vResults.push_back(SGTScriptParam(m_iFocusWin));
						vResults.push_back(SGTScriptParam(std::string(ac)));
						m_CallbackScript.CallFunction(m_mWindowInfos.find(m_iFocusWin)->second.strKeyPressCallback, vResults);
					}
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
		m_fXPos+=msg.mData.GetInt("ROT_X_REL")*m_fFactor;
		m_fYPos+=msg.mData.GetInt("ROT_Y_REL")*m_fFactor;
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
		std::map<int, SWindowInfo>::const_iterator it=m_mWindowInfos.begin();
		for(; it!=m_mWindowInfos.end(); it++)
			if(m_fXPos>it->second.x && m_fXPos<it->second.x+it->second.w &&
				m_fYPos>it->second.y && m_fYPos<it->second.y+it->second.h && it->first!=m_iCursorHandle)
				break;
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
		}

		if(iCurrHover!=m_iHoverWin)
		{
			if(iCurrHover!=-1)
				if(it->second.strHoverInCallback.size())
				{
					std::vector<SGTScriptParam> parms(1, SGTScriptParam(it->first));
					m_CallbackScript.CallFunction(it->second.strHoverInCallback, parms);
				}
			if(m_iHoverWin!=-1)
				if(m_mWindowInfos.find(m_iHoverWin)->second.strHoverOutCallback.size())
				{
					std::vector<SGTScriptParam> parms(1, m_iHoverWin);
					m_CallbackScript.CallFunction(m_mWindowInfos.find(m_iHoverWin)->second.strHoverOutCallback, parms);
				}
		}
		m_iHoverWin=iCurrHover;
	}
	if(msg.mNewsgroup == "UPDATE_PER_FRAME" && m_CallbackScript.GetID()==-1)
	{
		m_CallbackScript=SGTScriptSystem::GetInstance().CreateInstance("gui-hack.lua");
	}



	if(msg.mNewsgroup == "MOUSE_DOWN")
		if(OIS::MB_Left==msg.mData.GetInt("MOUSE_ID"))
		{
			std::map<int, SWindowInfo>::const_iterator it=m_mWindowInfos.begin();
			for(; it!=m_mWindowInfos.end(); it++)
				if(m_fXPos>it->second.x && m_fXPos<it->second.x+it->second.w &&
					m_fYPos>it->second.y && m_fYPos<it->second.y+it->second.h && it->first!=m_iCursorHandle)
					break;

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
				if(it->second.strOnClickCallback.size())
				{
					std::vector<SGTScriptParam> parms(1, SGTScriptParam(it->first));
					m_CallbackScript.CallFunction(it->second.strOnClickCallback, parms);
				}
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
SGTGUISystem::MakeWindow(float x, float y, float w, float h)
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
	wininfo.iDepth=1;
	m_mWindowInfos.insert(std::pair<int, SWindowInfo>(iHandle, wininfo));
	return Window(iHandle);
}


void
SGTGUISystem::Window::Bake()
{
	SGTGUISystem::SWindowInfo wininfo=SGTGUISystem::GetInstance().m_mWindowInfos.find(m_iHandle)->second;
	float x=wininfo.x, y=wininfo.y, w=wininfo.w, h=wininfo.h;
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
	afVertexData[8]=1.0f;
	afVertexData[9]=0.0f;

	afVertexData[10]=w;
	afVertexData[11]=h;
	afVertexData[12]=0.0f;
	afVertexData[13]=1.0f;
	afVertexData[14]=1.0f;

	afVertexData[15]=0.0f;
	afVertexData[16]=h;
	afVertexData[17]=0.0f;
	afVertexData[18]=0.0f;
	afVertexData[19]=1.0f;
	vbuf->unlock();

	//now the sub-windows
	int iSubWin=0;
	for(iSubWin=0; iSubWin<nSubWindows; iSubWin++)
	{
		SGTGUISystem::SWindowInfo subwininfo=SGTGUISystem::GetInstance().m_mWindowInfos.find(vSubWindows[iSubWin])->second;
		//get z level of this sub-window
		int iLevel=1;
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
		subwininfo.x+subwininfo.w,subwininfo.y,z,1.0,0.0,
		subwininfo.x,subwininfo.y+subwininfo.h,z,0.0,1.0,
		subwininfo.x+subwininfo.w,subwininfo.y+subwininfo.h,z,1.0,1.0};
		vbuf->writeData((iSubWin+1)*4*decl->getVertexSize(0), 4*decl->getVertexSize(0), afSubWinVerts, false);
	}

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
		SGTGUISystem::SWindowInfo subwininfo=SGTGUISystem::GetInstance().m_mWindowInfos.find(vSubWindows[iSubWin])->second;
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
	SGTMain::Instance().GetOgreSceneMgr()->getEntity(wininfo.strName)->getSubEntity(0)->setCustomParameter(0, Ogre::Vector4(x, y, 0, 0));
	for(iSubWin=0; iSubWin<nSubWindows; iSubWin++)
		SGTMain::Instance().GetOgreSceneMgr()->getEntity(wininfo.strName)->getSubEntity(SGTGUISystem::GetInstance().m_mWindowInfos.find(vSubWindows[iSubWin])->second.strName)->setCustomParameter(0, Ogre::Vector4(x, y, 0, 0));
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

void SGTGUISystem::Window::SetOnClickCallback(std::string strCallback){SGTGUISystem::GetInstance().m_mWindowInfos.find(m_iHandle)->second.strOnClickCallback=strCallback;}
void SGTGUISystem::Window::SetHoverInCallback(std::string strCallback){SGTGUISystem::GetInstance().m_mWindowInfos.find(m_iHandle)->second.strHoverInCallback=strCallback;}
void SGTGUISystem::Window::SetHoverOutCallback(std::string strCallback){SGTGUISystem::GetInstance().m_mWindowInfos.find(m_iHandle)->second.strHoverOutCallback=strCallback;}
void SGTGUISystem::Window::SetKeyCallback(std::string strCallback){SGTGUISystem::GetInstance().m_mWindowInfos.find(m_iHandle)->second.strKeyPressCallback=strCallback;}

void SGTGUISystem::SubWindow::SetOnClickCallback(std::string strCallback){SGTGUISystem::GetInstance().m_mWindowInfos.find(m_iHandle)->second.strOnClickCallback=strCallback;}
void SGTGUISystem::SubWindow::SetHoverInCallback(std::string strCallback){SGTGUISystem::GetInstance().m_mWindowInfos.find(m_iHandle)->second.strHoverInCallback=strCallback;}
void SGTGUISystem::SubWindow::SetHoverOutCallback(std::string strCallback){SGTGUISystem::GetInstance().m_mWindowInfos.find(m_iHandle)->second.strHoverOutCallback=strCallback;}
void SGTGUISystem::SubWindow::SetKeyCallback(std::string strCallback){SGTGUISystem::GetInstance().m_mWindowInfos.find(m_iHandle)->second.strKeyPressCallback=strCallback;}

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

void
SGTGUISystem::Window::SetMaterial(Ogre::String strMat)
{
	SGTMain::Instance().GetOgreSceneMgr()->getEntity(SGTGUISystem::GetInstance().m_mWindowInfos.find(m_iHandle)->second.strName)->getSubEntity("windowface")->setMaterialName(strMat);
}

void
SGTGUISystem::SubWindow::SetMaterial(Ogre::String strMat)
{
	SGTGUISystem::SWindowInfo wininfo=SGTGUISystem::GetInstance().m_mWindowInfos.find(m_iHandle)->second;
	Ogre::String strSubMeshName=wininfo.strName;
	while(wininfo.iParentHandle!=-1)
		wininfo=SGTGUISystem::GetInstance().m_mWindowInfos.find(wininfo.iParentHandle)->second;
	//Ogre::Mesh* pMesh=Ogre::MeshManager::getSingleton().getByName(wininfo.strName);
	///pMesh->getSubMesh(strSubMeshName)->setMaterialName(strMat);
	SGTMain::Instance().GetOgreSceneMgr()->getEntity(wininfo.strName)->getSubEntity(strSubMeshName)->setMaterialName(strMat);//getMesh()->getSubMesh(strSubMeshName)->setMaterialName(strMat);
}


SGTGUISystem::SubWindow
SGTGUISystem::CreateSubWindow(float x, float y, float w, float h, int iParentHandle)
{
	int iHandle=SGTSceneManager::Instance().RequestID();
	m_mWindowInfos.find(iParentHandle)->second.vSubWindows.push_back(iHandle);
	SWindowInfo parenthandle=m_mWindowInfos.find(iParentHandle)->second;
	SWindowInfo wininfo;
	wininfo.x=x*parenthandle.w;
	wininfo.y=y*parenthandle.h;
	wininfo.w=w*parenthandle.w;
	wininfo.h=h*parenthandle.h;
	wininfo.iParentHandle=iParentHandle;
	wininfo.strName=Ogre::String("GUI-System-Subwindow ") + SGTSceneManager::Instance().RequestIDStr();
	m_mWindowInfos.insert(std::pair<int, SWindowInfo>(iHandle, wininfo));
	return SubWindow(iHandle);
}

SGTGUISystem::Window::Window(int iHandle){m_iHandle=iHandle;}
SGTGUISystem::Window::Window(){m_iHandle=-1;}
SGTGUISystem::SubWindow::SubWindow(int iHandle){m_iHandle=iHandle;}

void
SGTGUISystem::Window::Move(float x, float y)
{
	SGTGUISystem::GetInstance().m_mWindowInfos.find(m_iHandle)->second.x=x;
	SGTGUISystem::GetInstance().m_mWindowInfos.find(m_iHandle)->second.y=y;
	SWindowInfo wi=SGTGUISystem::GetInstance().m_mWindowInfos.find(m_iHandle)->second;
	SGTMain::Instance().GetOgreSceneMgr()->getEntity(wi.strName)->getSubEntity(0)->setCustomParameter(0, Ogre::Vector4(wi.x, wi.y, 0, 0));
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
	if(m_iCursorHandle!=iHandle)
		m_lZOrder.remove(iHandle);
	if(m_iCursorHandle!=-1)
		m_lZOrder.pop_front();
	m_lZOrder.push_front(iHandle);
	if(m_iCursorHandle!=-1 && m_iCursorHandle!=iHandle)
		m_lZOrder.push_front(m_iCursorHandle);
	int iCurrDepth=0;
	for(std::list<int>::iterator it=m_lZOrder.begin(); it!=m_lZOrder.end(); it++)
	{
		SWindowInfo wininfo=m_mWindowInfos.find(*it)->second;
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
SGTGUISystem::SetFocus(int iHandle)
{
	m_iFocusWin=iHandle;
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