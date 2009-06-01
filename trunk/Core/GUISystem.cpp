#include "GUISystem.h"
#include <windows.h>
#include "SGTInput.h"


const float SGTGUISystem::m_fFactor=0.001f;
const float SGTGUISystem::cfFrameWidth=0.02f;

SGTGUISystem::SGTGUISystem(void)
{
	SGTMessageSystem::Instance().JoinNewsgroup(this, "MOUSE_MOVE");
	SGTMessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
	SGTMessageSystem::Instance().JoinNewsgroup(this, "MOUSE_DOWN");
	SGTMessageSystem::Instance().JoinNewsgroup(this, "KEY_DOWN");
	Ogre::SceneNode* pNode=SGTMain::Instance().GetOgreSceneMgr()->getRootSceneNode()->createChildSceneNode("SGTGuiSystemNode");
	pNode->setPosition(0, 0, 0);
	m_fXPos=m_fYPos=0.5f;
	m_wMouse=MakeWindow(m_fXPos, m_fYPos, 0.05, 0.05, true);
	m_wMouse.Bake();
	/*Window w1=MakeWindow(0.0, 0.0, 0.25, 0.25);
	
	SGTGUISystem::SubWindow sw=CreateSubWindow(0.5, 0.25, 0.5, 0.5, w1.GetHandle());
	CreateSubWindow(0.0, 0.25, 0.5, 0.5, w1.GetHandle());
	SGTGUISystem::SubWindow ssw=CreateSubWindow(0.25, 0.25, 0.5, 0.5, sw.GetHandle());
	Window w2=MakeWindow(0.25, 0.0, 0.25, 0.5);
	
	w1.Bake();
	w2.Bake();

	sw.SetMaterial("gui/subwin-test0");

	sw.SetHoverInCallback("HoverInCallback");
	sw.SetHoverOutCallback("HoverOutCallback");
	sw.SetOnClickCallback("ClickCallback");
	*/
	Window wExitMenu=MakeWindow(0.25, 0.25, 0.5, 0.5);
	SGTGUISystem::SubWindow wContinue=CreateSubWindow(0.25, 0.12, 0.5, 0.20, wExitMenu.GetHandle());
	SGTGUISystem::SubWindow wStart=CreateSubWindow(0.25, 0.37, 0.5, 0.20, wExitMenu.GetHandle());
	SGTGUISystem::SubWindow wQuit=CreateSubWindow(0.25, 0.63, 0.5, 0.20, wExitMenu.GetHandle());
		
	wExitMenu.Bake();

	wContinue.SetMaterial("gui/ResumeGame");
	wStart.SetMaterial("gui/StartGame");
	wQuit.SetMaterial("gui/QuitGame");

	wContinue.SetHoverInCallback("ResumeHoverInCallback");
	wContinue.SetHoverOutCallback("ResumeHoverOutCallback");
	wContinue.SetOnClickCallback("ResumeClickCallback");

	wStart.SetHoverInCallback("StartHoverInCallback");
	wStart.SetHoverOutCallback("StartHoverOutCallback");
	wStart.SetOnClickCallback("StartClickCallback");

	wQuit.SetHoverInCallback("QuitHoverInCallback");
	wQuit.SetHoverOutCallback("QuitHoverOutCallback");
	wQuit.SetOnClickCallback("QuitClickCallback");
	
	
	SGTScriptSystem::GetInstance().ShareCFunction("set_window_material", Window::Lua_SetMaterial);
	m_iHoverWin=-1;
	m_bMenuActive=true;
}

SGTGUISystem::~SGTGUISystem(void)
{
}

void SGTGUISystem::ReceiveMessage(SGTMsg &msg)
{
	if(msg.mNewsgroup == "KEY_DOWN")
	{
		if(OIS::KC_ESCAPE==msg.mData.GetInt("KEY_ID_OIS"))
			m_bMenuActive=!m_bMenuActive;
		std::map<int, SWindowInfo>::const_iterator it=m_mWindowInfos.begin();
		for(; it!=m_mWindowInfos.end(); it++)
			if(it->second.iParentHandle==-1)
				SGTMain::Instance().GetOgreSceneMgr()->getEntity(it->second.strName)->setVisible(m_bMenuActive);
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
		m_wMouse.Move(m_fXPos, m_fYPos);
		std::map<int, SWindowInfo>::const_iterator it=m_mWindowInfos.begin();
		for(; it!=m_mWindowInfos.end(); it++)
			if(m_fXPos>it->second.x && m_fXPos<it->second.x+it->second.w &&
				m_fYPos>it->second.y && m_fYPos<it->second.y+it->second.h && it!=m_mWindowInfos.find(m_wMouse.GetHandle()))
				break;
		//mouse is somewhere inside this window, trace it down
		int iCurrHover=-1;
		if(it!=m_mWindowInfos.end())
		{
			for(unsigned int iSubWin=0; iSubWin<it->second.vSubWindows.size(); iSubWin++)
				if(m_fXPos>m_mWindowInfos.find(it->second.vSubWindows[iSubWin])->second.x && m_fXPos<m_mWindowInfos.find(it->second.vSubWindows[iSubWin])->second.x+m_mWindowInfos.find(it->second.vSubWindows[iSubWin])->second.w &&
					m_fYPos>m_mWindowInfos.find(it->second.vSubWindows[iSubWin])->second.y && m_fYPos<m_mWindowInfos.find(it->second.vSubWindows[iSubWin])->second.y+m_mWindowInfos.find(it->second.vSubWindows[iSubWin])->second.h)
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
					m_fYPos>it->second.y && m_fYPos<it->second.y+it->second.h && it!=m_mWindowInfos.find(m_wMouse.GetHandle()))
					break;

			//mouse is somewhere inside this window, trace it down
			if(it!=m_mWindowInfos.end())
			{
				for(unsigned int iSubWin=0; iSubWin<it->second.vSubWindows.size(); iSubWin++)
					if(m_fXPos>m_mWindowInfos.find(it->second.vSubWindows[iSubWin])->second.x && m_fXPos<m_mWindowInfos.find(it->second.vSubWindows[iSubWin])->second.x+m_mWindowInfos.find(it->second.vSubWindows[iSubWin])->second.w &&
						m_fYPos>m_mWindowInfos.find(it->second.vSubWindows[iSubWin])->second.y && m_fYPos<m_mWindowInfos.find(it->second.vSubWindows[iSubWin])->second.y+m_mWindowInfos.find(it->second.vSubWindows[iSubWin])->second.h)
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
SGTGUISystem::MakeWindow(float x, float y, float w, float h, bool bPopUp)
{
	int iHandle=SGTSceneManager::Instance().RequestID();
	Window win(iHandle);
	SWindowInfo wininfo;
	wininfo.bPopUp=bPopUp;
	wininfo.iParentHandle=-1;
	wininfo.strName=Ogre::String("GUI-System-Window ") + SGTSceneManager::Instance().RequestIDStr();
	wininfo.x=x;
	wininfo.y=y;
	wininfo.w=w;
	wininfo.h=h;
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

	if(wininfo.bPopUp)
	{
		int nTris=2;
		int nVerts=4;
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
		afVertexData[4]=1.0f;

		afVertexData[5]=2.0*w;
		afVertexData[6]=0.0f;
		afVertexData[7]=0.0f;
		afVertexData[8]=1.0f;
		afVertexData[9]=1.0f;

		afVertexData[10]=2.0*w;
		afVertexData[11]=2.0*h;
		afVertexData[12]=0.0f;
		afVertexData[13]=1.0f;
		afVertexData[14]=0.0f;

		afVertexData[15]=0.0f;
		afVertexData[16]=2.0*h;
		afVertexData[17]=0.0f;
		afVertexData[18]=0.0f;
		afVertexData[19]=0.0f;
		vbuf->unlock();
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

		meshptr.get()->getSubMesh("windowface")->indexData->indexBuffer = ibuf;
		meshptr.get()->getSubMesh("windowface")->indexData->indexCount = nTris*3;
		meshptr.get()->getSubMesh("windowface")->indexData->indexStart = 0;
		meshptr.get()->getSubMesh("windowface")->setMaterialName("gui/popup-test");
	}
	else
	{
		std::vector<int> vSubWindows=GetSubWindows();
		int nSubWindows=vSubWindows.size();
		int nTris=18+nSubWindows*2;
		int nVerts=16+nSubWindows*4;
		Ogre::VertexData* data = new Ogre::VertexData();
		meshptr.get()->sharedVertexData = data;
		data->vertexCount = nVerts;
		Ogre::VertexDeclaration* decl = data->vertexDeclaration;
		decl->addElement(0, 0, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
		size_t off = Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
		decl->addElement(0, off, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES);
		Ogre::HardwareVertexBufferSharedPtr vbuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
		decl->getVertexSize(0), nVerts, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);

		float afVertices[]={
		0.0,0.0,0.0,0.0,1.0,
		cfFrameWidth,0.0,0.0,1.0/3.0,1.0,
		0.0,cfFrameWidth,0.0,0.0,2.0/3.0,
		cfFrameWidth,cfFrameWidth,0.0,1.0/3.0,2.0/3.0,

		w*2.0-cfFrameWidth,0.0,0.0,2.0/3.0,1.0,
		w*2.0,0.0,0.0,1.0,1.0,
		w*2.0-cfFrameWidth,cfFrameWidth,0.0,2.0/3.0,2.0/3.0,
		w*2.0,cfFrameWidth,0.0,1.0,2.0/3.0,
		
		0.0,h*2.0-cfFrameWidth,0.0,0.0,1.0/3.0,
		cfFrameWidth,h*2.0-cfFrameWidth,0.0,1.0/3.0,1.0/3.0,
		0.0,h*2.0,0.0,0.0,0.0,
		cfFrameWidth,h*2.0,0.0,1.0/3.0,0.0,
		
		w*2.0-cfFrameWidth,h*2.0-cfFrameWidth,0.0,2.0/3.0,1.0/3.0,
		w*2.0,h*2.0-cfFrameWidth,0.0,1.0,1.0/3.0,
		w*2.0-cfFrameWidth,h*2.0,0.0,2.0/3.0,0.0,
		w*2.0,h*2.0,0.0,1.0,0.0};

		vbuf->writeData(0, 16*decl->getVertexSize(0), afVertices, true);

		//now the sub-windows

		int iSubWin=0;
		for(iSubWin=0; iSubWin<nSubWindows; iSubWin++)
		{
			SGTGUISystem::SWindowInfo subwininfo=SGTGUISystem::GetInstance().m_mWindowInfos.find(vSubWindows[iSubWin])->second;
			float z=0.0001-0.00009*(float)(iSubWin+1)/(float)nSubWindows;
			float afSubWinVerts[]={
			(subwininfo.x)*2.0,subwininfo.y*2.0,z,0.0,1.0,
			(subwininfo.x+subwininfo.w)*2.0,subwininfo.y*2.0,z,1.0,1.0,
			subwininfo.x*2.0,(subwininfo.y+subwininfo.h)*2.0,z,0.0,0.0,
			(subwininfo.x+subwininfo.w)*2.0,(subwininfo.y+subwininfo.h)*2.0,z,1.0,0.0};
			vbuf->writeData((16+iSubWin*4)*decl->getVertexSize(0), 4*decl->getVertexSize(0), afSubWinVerts, false);
		}

		Ogre::VertexBufferBinding* bind = data->vertexBufferBinding;
		bind->setBinding(0, vbuf);

		Ogre::HardwareIndexBufferSharedPtr ibuf = Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
		Ogre::HardwareIndexBuffer::IT_16BIT, nTris*3, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
			
		unsigned short aiIndices[]=
		{0,1,2, 1,3,2,
		4,5,6, 5,7,6,
		8,9,10, 9,11,10,
		12,13,14, 13,15,14,
		
		1,4,6, 1,6,3,
		3,9,8, 3,8,2,
		9,12,14, 9,14,11,
		12,6,7, 12,7,13,
		3,6,12, 3,12,9};

		ibuf->writeData(0, sizeof(unsigned short)*54, aiIndices, true);

		for(iSubWin=0; iSubWin<nSubWindows; iSubWin++)
		{
			SGTGUISystem::SWindowInfo subwininfo=SGTGUISystem::GetInstance().m_mWindowInfos.find(vSubWindows[iSubWin])->second;
			unsigned short aiSubWinIndices[]={
				16+4*iSubWin, 16+4*iSubWin+1, 16+4*iSubWin+2,
				16+4*iSubWin+1, 16+4*iSubWin+3, 16+4*iSubWin+2,
			};
			ibuf->writeData(sizeof(unsigned short)*(54 + 6*iSubWin), 6*sizeof(unsigned short), aiSubWinIndices, false);
			Ogre::SubMesh* pSM=meshptr.get()->createSubMesh(subwininfo.strName);
			pSM->indexData->indexBuffer = ibuf;
			pSM->indexData->indexCount = 2*3;
			pSM->indexData->indexStart = 18*3+2*3*iSubWin;
			pSM->setMaterialName("gui/subwin-test");
		}

		meshptr.get()->getSubMesh("windowface")->indexData->indexBuffer = ibuf;
		meshptr.get()->getSubMesh("windowface")->indexData->indexCount = 18*3;
		meshptr.get()->getSubMesh("windowface")->indexData->indexStart = 0;
		meshptr.get()->getSubMesh("windowface")->setMaterialName("gui/win-test");
	}
	

	float fInf=1000000000.0f;
	meshptr.get()->_setBounds(Ogre::AxisAlignedBox(-fInf,-fInf,-fInf,fInf,fInf,fInf));
	meshptr.get()->_setBoundingSphereRadius(fInf);		
	meshptr.get()->load();
	SGTMain::Instance().GetOgreSceneMgr()->getSceneNode("SGTGuiSystemNode")->attachObject(SGTMain::Instance().GetOgreSceneMgr()->createEntity(wininfo.strName, wininfo.strName));
	SGTMain::Instance().GetOgreSceneMgr()->getEntity(wininfo.strName)->getSubEntity(0)->setCustomParameter(0, Ogre::Vector4(x, y, 0, 0));

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

void SGTGUISystem::SubWindow::SetOnClickCallback(std::string strCallback){SGTGUISystem::GetInstance().m_mWindowInfos.find(m_iHandle)->second.strOnClickCallback=strCallback;}
void SGTGUISystem::SubWindow::SetHoverInCallback(std::string strCallback){SGTGUISystem::GetInstance().m_mWindowInfos.find(m_iHandle)->second.strHoverInCallback=strCallback;}
void SGTGUISystem::SubWindow::SetHoverOutCallback(std::string strCallback){SGTGUISystem::GetInstance().m_mWindowInfos.find(m_iHandle)->second.strHoverOutCallback=strCallback;}

std::vector<SGTScriptParam>
SGTGUISystem::Window::Lua_SetMaterial(SGTScript& caller, std::vector<SGTScriptParam> vParams)
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

void
SGTGUISystem::Window::SetMaterial(Ogre::String strMat)
{
	SGTMain::Instance().GetOgreSceneMgr()->getEntity(SGTGUISystem::GetInstance().m_mWindowInfos.find(m_iHandle)->second.strName)->getMesh()->getSubMesh("windowface")->setMaterialName(strMat);
	//Ogre::MeshPtr meshptr=Ogre::MeshManager::getSingleton().getByName(SGTGUISystem::GetInstance().m_mWindowInfos.find(m_iHandle)->second.strName)->;
	//pMesh->getSubMesh("windowface")->setMaterialName(strMat);
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
	wininfo.x=parenthandle.x+x*parenthandle.w;
	wininfo.y=parenthandle.y+y*parenthandle.h;
	wininfo.w=w*parenthandle.w;
	wininfo.h=h*parenthandle.h;
	wininfo.iParentHandle=iParentHandle;
	wininfo.bPopUp=false;
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