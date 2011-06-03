
#include "VertexMultitextureWeightBrusher.h"
#include "EntityMaterialInspector.h"

unsigned short VertexMultitextureWeightBrusher::EnsureHasMultitextureWeightBuffer(Ogre::VertexData *vertexData)
{
	for (unsigned int i = 0; i < vertexData->vertexDeclaration->getElementCount(); i++)
	{
		const Ogre::VertexElement *elem = vertexData->vertexDeclaration->getElement(i);
		if (elem->getSemantic() == Ogre::VertexElementSemantic::VES_DIFFUSE && elem->getIndex() == 0) return elem->getSource();
	}

	int bufferIndex = vertexData->vertexBufferBinding->getBufferCount();
	vertexData->vertexDeclaration->addElement(bufferIndex, 0, Ogre::VertexElementType::VET_COLOUR_ARGB, Ogre::VertexElementSemantic::VES_DIFFUSE, 0);
	Ogre::HardwareVertexBufferSharedPtr vWeightBuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
		Ogre::VertexElement::getTypeSize(Ogre::VertexElementType::VET_COLOUR_ARGB), vertexData->vertexCount, Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE);

	char *oneBuff = new char[vWeightBuf->getSizeInBytes()];
	memset(oneBuff, 0, vWeightBuf->getSizeInBytes());
	vWeightBuf->writeData(0, vWeightBuf->getSizeInBytes(), oneBuff, true);
	delete oneBuff;

	vertexData->vertexBufferBinding->setBinding(bufferIndex, vWeightBuf);

	return bufferIndex;
}

void VertexMultitextureWeightBrusher::EnsureHasMultitextureWeightBuffer(Ogre::MeshPtr mesh)
{
	if (mesh->sharedVertexData) EnsureHasMultitextureWeightBuffer(mesh->sharedVertexData);

	for (unsigned int i = 0; i < mesh->getNumSubMeshes(); i++)
	{
		Ogre::SubMesh *subMesh = mesh->getSubMesh(i);
		if (subMesh->vertexData) EnsureHasMultitextureWeightBuffer(subMesh->vertexData);
	}
}

void VertexMultitextureWeightBrusher::ProcessVertexData(Ogre::VertexData *vertexData, Ogre::Vector3 brushPos, float radius, unsigned int textureLayer)
{
	const Ogre::VertexElement* posElem =
		vertexData->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
	Ogre::HardwareVertexBufferSharedPtr vPosBuf =
		vertexData->vertexBufferBinding->getBuffer(posElem->getSource());
		unsigned char* pPosVertices = static_cast<unsigned char*>(vPosBuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
	Ogre::HardwareVertexBufferSharedPtr vWeightBuf =
		vertexData->vertexBufferBinding->getBuffer(EnsureHasMultitextureWeightBuffer(vertexData));
	unsigned char* pWeightVertices = static_cast<unsigned char*>(vWeightBuf->lock(Ogre::HardwareBuffer::HBL_NORMAL));

	Ogre::Real *pPos;
	unsigned char *pWeight;
	float squaredRadius = radius*radius;
	for (unsigned int i = 0; i < vertexData->vertexCount; i++)
	{
		posElem->baseVertexPointerToElement(pPosVertices, &pPos);
		Ogre::Vector3 pos;
		pos.x = (*pPos++);
		pos.y = (*pPos++);
		pos.z = (*pPos++);
		float squaredDist = pos.squaredDistance(brushPos);
		if (squaredDist < squaredRadius)
		{
			float dist = Ogre::Math::Sqrt(squaredDist);
			posElem->baseVertexPointerToElement(pWeightVertices, &pWeight);
			float weight = 1-(dist/radius);
			pWeight[textureLayer] = (char)255;
		}
		pPosVertices += vPosBuf->getVertexSize();
		pWeightVertices += vWeightBuf->getVertexSize();
	}
	vPosBuf->unlock();
	vWeightBuf->unlock();
}

void VertexMultitextureWeightBrusher::SetMultitextureWeight(Ogre::MeshPtr mesh, Ogre::SubMesh *subMesh, Ogre::Vector3 brushPos, float radius, unsigned int textureLayer)
{
	IceAssert(textureLayer <= 4)

	EnsureHasMultitextureWeightBuffer(mesh);

	if (mesh->sharedVertexData) ProcessVertexData(mesh->sharedVertexData, brushPos, radius, textureLayer);
	if (subMesh->vertexData) ProcessVertexData(subMesh->vertexData, brushPos, radius, textureLayer);
}

void VertexMultitextureWeightBrusher::SetMultitextureWeight(Ogre::Entity *entity, Ogre::Ray ray, float radius, unsigned int textureLayer)
{
	EntityMaterialInspector emi(entity, ray);
	Ogre::SubEntity *subEnt = emi.GetSubEntity();
	if (!subEnt) return;
	SetMultitextureWeight(entity->getMesh(), subEnt->getSubMesh(), emi.GetHitPosition() - entity->getParentNode()->_getDerivedPosition(), radius, textureLayer);
}