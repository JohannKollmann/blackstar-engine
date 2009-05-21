#include "GUISystem.h"
//#include <windows.h>


const float SGTGUISystem::m_fFactor=0.001f;

SGTGUISystem::SGTGUISystem(void)
{
	SGTMessageSystem::Instance().JoinNewsgroup(this, "MOUSE_MOVE");
	m_bMeshInitialized=false;	
	SGTMessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
}

SGTGUISystem::~SGTGUISystem(void)
{
}

void SGTGUISystem::ReceiveMessage(SGTMsg &msg)
{
	if (msg.mNewsgroup == "MOUSE_MOVE")
	{
		m_fXPos+=msg.mData.GetInt("ROT_X_REL")*m_fFactor;
		m_fYPos+=msg.mData.GetInt("ROT_X_REL")*m_fFactor;
		//MessageBox(0, "", "", 0);
		if(m_fXPos>1.0)
			m_fXPos=1.0;
		if(m_fXPos<0.0)
			m_fXPos=0.0;
		if(m_fYPos>1.0)
			m_fYPos=1.0;
		if(m_fYPos<0.0)
			m_fYPos=0.0;
	}
	if(msg.mNewsgroup == "UPDATE_PER_FRAME" && !m_bMeshInitialized)
	{
		m_fYPos=m_fXPos=0.5f;
		Ogre::MeshPtr meshptr = Ogre::MeshManager::getSingleton().createManual("myCanvas", "General");
		meshptr.get()->createSubMesh("1");

		Ogre::VertexData* data = new Ogre::VertexData();
		meshptr.get()->sharedVertexData = data;
		data->vertexCount = 4;
		Ogre::VertexDeclaration* decl = data->vertexDeclaration;
		decl->addElement(0, 0, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
		size_t off = Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
		decl->addElement(0, off, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES);
		Ogre::HardwareVertexBufferSharedPtr vbuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
		decl->getVertexSize(0), 4, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);

		float* afVertexData=(float*)vbuf->lock(Ogre::HardwareBuffer::HBL_DISCARD);
		afVertexData[0]=-10.0f;
		afVertexData[1]=-10.0f;
		afVertexData[2]=0.0f;
		afVertexData[3]=0.0f;
		afVertexData[4]=0.0f;

		afVertexData[5]=10.0f;
		afVertexData[6]=-10.0f;
		afVertexData[7]=0.0f;
		afVertexData[8]=1.0f;
		afVertexData[9]=0.0f;

		afVertexData[10]=10.0f;
		afVertexData[11]=10.0f;
		afVertexData[12]=0.0f;
		afVertexData[13]=1.0f;
		afVertexData[14]=1.0f;

		afVertexData[15]=-10.0f;
		afVertexData[16]=10.0f;
		afVertexData[17]=0.0f;
		afVertexData[18]=0.0f;
		afVertexData[19]=1.0f;
		vbuf->unlock();
		Ogre::VertexBufferBinding* bind = data->vertexBufferBinding;
		bind->setBinding(0, vbuf);

		Ogre::HardwareIndexBufferSharedPtr ibuf = Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
		Ogre::HardwareIndexBuffer::IT_16BIT, 6, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
		
		unsigned short* aiIndexBuf=(unsigned short*)ibuf->lock(Ogre::HardwareBuffer::HBL_DISCARD);
		aiIndexBuf[0]=0;
		aiIndexBuf[1]=2;
		aiIndexBuf[2]=1;
		aiIndexBuf[3]=0;
		aiIndexBuf[4]=3;
		aiIndexBuf[5]=2;
		ibuf->unlock();

		meshptr.get()->getSubMesh("1")->indexData->indexBuffer = ibuf;
		meshptr.get()->getSubMesh("1")->indexData->indexCount = 6;
		meshptr.get()->getSubMesh("1")->indexData->indexStart = 0;
		
		Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().create("myMaterial", "General", true);
		mat->getTechnique(0)->getPass(0)->createTextureUnitState();
		mat->getTechnique(0)->getPass(0)->setDiffuse(Ogre::ColourValue(0, 255, 0, 255));
		

		mat->setLightingEnabled(false);
		mat->setDepthCheckEnabled(false);
		mat->getTechnique(0)->getPass(0)->setCullingMode(Ogre::CULL_NONE);
		//mat->getTechnique(0)->getPass(0)->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
		mat->load();

		meshptr.get()->getSubMesh("1")->setMaterialName("myMaterial");
		meshptr.get()->_setBounds(Ogre::AxisAlignedBox(-10,-10,-1,10,10,1));
		meshptr.get()->_setBoundingSphereRadius(15);


		meshptr.get()->load();
		Ogre::SceneNode* myNode = SGTMain::Instance().GetOgreSceneMgr()->getRootSceneNode()->createChildSceneNode();
		myNode->setPosition(0, 0, 0);
		myNode->attachObject(SGTMain::Instance().GetOgreSceneMgr()->createEntity("bleh", "myCanvas"));
		m_bMeshInitialized=true;
	}
}