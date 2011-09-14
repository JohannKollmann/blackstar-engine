
#include "AmbientOcclusionGenerator.h"
#include "OgrePhysX.h"
#include "IceMain.h"
#include "IceGOCPhysics.h"
#include "IceMainLoop.h"
#include "wxEdit.h"


AmbientOcclusionGenerator::AmbientOcclusionGenerator(void)
{
}


AmbientOcclusionGenerator::~AmbientOcclusionGenerator(void)
{
}

AmbientOcclusionGenerator& AmbientOcclusionGenerator::Instance()
{
	static AmbientOcclusionGenerator generator;
	return generator;
}

void AmbientOcclusionGenerator::bakeAmbientOcclusion(Ogre::MeshPtr mesh, Ogre::String outputFile)
{
	wxEdit::Instance().GetProgressBar()->SetStatusMessage("Computing Ambient Occlusion");
	//wxEdit::Instance().GetProgressBar()->SetShowBar(true);

	wxEdit::Instance().GetProgressBar()->SetProgress(0.1f);
	wxEdit::Instance().GetAuiManager().Update();
	//for (int i = 0; i < 1000; i++) wxEdit::Instance().GetProgressBar()->SetProgress(0.1f);	//Hack

	OgrePhysX::Actor<PxRigidStatic> actor = Ice::Main::Instance().GetPhysXScene()->createRigidStatic(OgrePhysX::Geometry::triangleMeshGeometry(mesh, OgrePhysX::CookerParams().backfaces(true)));

	OgrePhysX::World::getSingleton().simulate(0.1f);

	wxEdit::Instance().GetProgressBar()->SetProgress(0.4f);

	Ogre::MeshPtr aoMesh = Ogre::MeshManager::getSingleton().createManual("TmpAOMesh", "General");

	if (mesh->sharedVertexData)
	{
		aoMesh->sharedVertexData = new Ogre::VertexData();
		processVertexData(aoMesh->sharedVertexData, mesh->sharedVertexData, 1<<Ice::CollisionGroups::TMP);
	}
	for (unsigned i = 0; i < mesh->getNumSubMeshes(); i++)
	{
		Ogre::SubMesh *subMesh = mesh->getSubMesh(i);
		Ogre::SubMesh *aoSubMesh = aoMesh->createSubMesh();
		aoSubMesh->useSharedVertices = subMesh->useSharedVertices;
		aoSubMesh->setMaterialName(subMesh->getMaterialName());
		//Copy index data
		aoSubMesh->indexData = subMesh->indexData->clone();
		if (subMesh->vertexData)
		{
			aoSubMesh->vertexData = new Ogre::VertexData();
			processVertexData(aoSubMesh->vertexData, subMesh->vertexData, 1<<Ice::CollisionGroups::TMP);
		}
		wxEdit::Instance().GetProgressBar()->SetProgress(0.4f + 0.6f * ((float)i/(float)mesh->getNumSubMeshes()));
	}

	aoMesh->_setBounds(mesh->getBounds());
	aoMesh->_setBoundingSphereRadius(mesh->getBoundingSphereRadius());

	Ogre::MeshSerializer ms;
	ms.exportMesh(aoMesh.getPointer(), outputFile);
	Ogre::MeshManager::getSingleton().remove(aoMesh->getHandle());

	Ice::Main::Instance().GetPhysXScene()->removeActor(actor);
	wxEdit::Instance().GetProgressBar()->Reset();
}

void AmbientOcclusionGenerator::bakeAmbientOcclusion(Ogre::Entity *ent, Ogre::String outputFile)
{
	wxEdit::Instance().GetProgressBar()->SetStatusMessage("Computing Ambient Occlusion");

	wxEdit::Instance().GetProgressBar()->SetProgress(0.1f);
	wxEdit::Instance().GetAuiManager().Update();

	Ogre::MeshPtr mesh = ent->getMesh();

	Ogre::MeshPtr aoMesh = Ogre::MeshManager::getSingleton().createManual("TmpAOMesh", "General");

	if (mesh->sharedVertexData)
	{
		aoMesh->sharedVertexData = new Ogre::VertexData();
		processVertexData(aoMesh->sharedVertexData, mesh->sharedVertexData, 1<<Ice::CollisionGroups::DEFAULT|1<<Ice::CollisionGroups::LEVELMESH, ent->getParentNode());
	}
	for (unsigned i = 0; i < mesh->getNumSubMeshes(); i++)
	{
		Ogre::SubMesh *subMesh = mesh->getSubMesh(i);
		Ogre::SubMesh *aoSubMesh = aoMesh->createSubMesh();
		aoSubMesh->useSharedVertices = subMesh->useSharedVertices;
		aoSubMesh->setMaterialName(subMesh->getMaterialName());
		//Copy index data
		aoSubMesh->indexData = subMesh->indexData->clone();
		if (subMesh->vertexData)
		{
			aoSubMesh->vertexData = new Ogre::VertexData();
			processVertexData(aoSubMesh->vertexData, subMesh->vertexData, 1<<Ice::CollisionGroups::DEFAULT|1<<Ice::CollisionGroups::LEVELMESH, ent->getParentNode());
		}
		wxEdit::Instance().GetProgressBar()->SetProgress(0.4f + 0.6f * ((float)i/(float)mesh->getNumSubMeshes()));
	}

	aoMesh->_setBounds(mesh->getBounds());
	aoMesh->_setBoundingSphereRadius(mesh->getBoundingSphereRadius());

	Ogre::MeshSerializer ms;
	ms.exportMesh(aoMesh.getPointer(), outputFile);
	Ogre::MeshManager::getSingleton().remove(aoMesh->getHandle());

	wxEdit::Instance().GetProgressBar()->Reset();
}

void AmbientOcclusionGenerator::processVertexData(Ogre::VertexData *targetData, Ogre::VertexData *inputData, int rayCollisionGroups, Ogre::Node *baseNode)
{
	targetData->vertexCount = inputData->vertexCount;
	Ogre::VertexDeclaration *vd = targetData->vertexDeclaration;

	//We will use a new buffer for the ao
	bool addedAODecl = false;
	int aoBufferSource = -1;

	std::map<int, int> vertexDeclsOffsets;

	for (unsigned int i = 0; i < inputData->vertexDeclaration->getElementCount(); i++)
	{
		const Ogre::VertexElement *elem = inputData->vertexDeclaration->getElement(i);
		if (vertexDeclsOffsets.find(elem->getSource()) == vertexDeclsOffsets.end()) vertexDeclsOffsets.insert(std::make_pair<int, int>(elem->getSource(), 0));
		vd->addElement(elem->getSource(), vertexDeclsOffsets[elem->getSource()], elem->getType(), elem->getSemantic());
		vertexDeclsOffsets[elem->getSource()] += Ogre::VertexElement::getTypeSize(elem->getType());

		if (elem->getSemantic() == Ogre::VertexElementSemantic::VES_DIFFUSE) aoBufferSource = elem->getSource();

		/*if (elem->getSemantic() == Ogre::VertexElementSemantic::VES_NORMAL)
		{
			//Add diffuse after normal
			//vd->addElement(aoBufferSource, 0, Ogre::VertexElementType::VET_COLOUR, Ogre::VertexElementSemantic::VES_DIFFUSE, 0); 
			//offset += Ogre::VertexElement::getTypeSize(Ogre::VertexElementType::VET_COLOUR);
		}*/
	}
	if (aoBufferSource == -1)
	{
		aoBufferSource = inputData->vertexBufferBinding->getBufferCount();
		vd->addElement(aoBufferSource, 0, Ogre::VertexElementType::VET_COLOUR_ARGB, Ogre::VertexElementSemantic::VES_DIFFUSE, 0); 
	}

	//Clone existing buffers
	for (unsigned int i = 0; i < inputData->vertexBufferBinding->getBufferCount(); i++)
	{
		if (i == aoBufferSource) continue;	//do not copy original ao buffer

		const Ogre::HardwareVertexBufferSharedPtr oldBuf = inputData->vertexBufferBinding->getBuffer(i);
		Ogre::HardwareVertexBufferSharedPtr vbuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
					oldBuf->getVertexSize(), oldBuf->getNumVertices(), Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
		vbuf->copyData(*oldBuf);
		targetData->vertexBufferBinding->setBinding(i, vbuf);
	}


	const Ogre::VertexElement* posElem =
		targetData->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
	const Ogre::VertexElement* normElem =
		targetData->vertexDeclaration->findElementBySemantic(Ogre::VES_NORMAL);
	Ogre::HardwareVertexBufferSharedPtr vPosBuf =
		targetData->vertexBufferBinding->getBuffer(posElem->getSource());
		unsigned char* pVertices = static_cast<unsigned char*>(vPosBuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
	Ogre::Real* pReal;

	//Compute AO and add ao buffer
	unsigned char *aoBuffer = new unsigned char[targetData->vertexCount*4];
	memset(aoBuffer, 0, targetData->vertexCount*4);
	Ogre::RenderSystem* rs = Ogre::Root::getSingleton().getRenderSystem();
	//AO Calculation
	for (unsigned int i = 0; i < targetData->vertexCount; i++)
	{
		posElem->baseVertexPointerToElement(pVertices, &pReal);
		Ogre::Vector3 pos;
		pos.x = (*pReal++);
		pos.y = (*pReal++);
		pos.z = (*pReal++);
		normElem->baseVertexPointerToElement(pVertices, &pReal);
		Ogre::Vector3 normal;
		normal.x = (*pReal++);
		normal.y = (*pReal++);
		normal.z = (*pReal++);
		pVertices += vPosBuf->getVertexSize();

		if (baseNode)
		{
			pos = baseNode->_getDerivedPosition() + baseNode->_getDerivedOrientation()*(pos*baseNode->_getDerivedScale());
			normal = baseNode->_getDerivedOrientation()*normal;
		}
		char aoFactor = (char)(computeAO(pos, normal, rayCollisionGroups) * 255);
		aoBuffer[i*4+3] = aoFactor;
		//rs->convertColourValue(Ogre::ColourValue(aoFactor, aoFactor, aoFactor, 1), &aoBuffer[i]);
	}
	vPosBuf->unlock();

	Ogre::HardwareVertexBufferSharedPtr vAOBuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
		Ogre::VertexElement::getTypeSize(Ogre::VertexElementType::VET_COLOUR_ARGB), targetData->vertexCount, Ogre::HardwareBuffer::HBU_DYNAMIC);
	vAOBuf->writeData(0, vAOBuf->getSizeInBytes(), aoBuffer, true);
	targetData->vertexBufferBinding->setBinding(aoBufferSource, vAOBuf);
	delete aoBuffer;
}

float AmbientOcclusionGenerator::computeAO(Ogre::Vector3 position, Ogre::Vector3 normal, int rayCollisionGroups)
{
	const float c = 4;
	float fNumSamples = 0;
	float sum = 0;

	for(float h=Ogre::Math::PI/4/c; h<Ogre::Math::PI/2; h+=Ogre::Math::PI/2/c)
	{
		for(float v=0; v<2*Ogre::Math::PI; v+=Ogre::Math::PI/2/(c*Ogre::Math::Cos(h)))
		{
			Ogre::Vector3 castVector = Ogre::Vector3(Ogre::Math::Cos(h)*Ogre::Math::Sin(v),Ogre::Math::Sin(h),Ogre::Math::Cos(h)*Ogre::Math::Cos(v));
			//Ice::Log::Instance().LogMessage(Ogre::StringConverter::toString(castVector));
			castVector.normalise();
			Ogre::Quaternion q = Ogre::Vector3::UNIT_Y.getRotationTo(normal);
			castVector = q * castVector;
			Ogre::Ray ray(position + (castVector * 0.001f), castVector);
			float test = Ogre::Vector3::UNIT_Y.dotProduct(castVector);		//normal.dotProduct(castVector);
			float weight = Ogre::Math::Abs(normal.dotProduct(castVector));
			if (test < 0)	
			{
				fNumSamples += 0.5f*weight;	//light is coming from below	
				continue;
			}
			//weight = 0.5f + 0.5f*weight;
			if (test >= 0 && !Ice::Main::Instance().GetPhysXScene()->raycastAny(position + (castVector * 0.001f), castVector, 20))//, 10))
				sum += 1*weight;
			fNumSamples += 1*weight;
		}
	}
	if (fNumSamples < 1) fNumSamples = 1;
	float avg = sum/fNumSamples;
	//avg *= 2;
	if (avg > 1) avg = 1;

	float aoOffset = 1-avg;
	return aoOffset;
	//if (aoOffset < 0.1f) aoOffset = 0.1f;
}
