

#include "EntityMaterialInspector.h"

EntityMaterialInspector::EntityMaterialInspector(Ogre::Entity *entity)
{
	mEntity = entity;
}

EntityMaterialInspector::~EntityMaterialInspector(void)
{
}


std::vector<SubMeshInformation*> EntityMaterialInspector::GetSubMeshes(const Ogre::Vector3 &position, const Ogre::Quaternion &orient, const Ogre::Vector3 &scale)
{
	SubMeshInformation* currentSubMesh;
	std::vector<SubMeshInformation*> returner; 
	std::vector<Ogre::Vector3> shared_vertices;

	Ogre::VertexData* shared_vertex_data = mEntity->getMesh()->sharedVertexData;
	if (shared_vertex_data != NULL)
	{
            const Ogre::VertexElement* posElem =
                shared_vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);

            Ogre::HardwareVertexBufferSharedPtr vbuf =
                shared_vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());

            unsigned char* vertex =
                static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

			float* pReal;

		for(size_t j = 0; j < shared_vertex_data->vertexCount; ++j, vertex += vbuf->getVertexSize())
        {
			posElem->baseVertexPointerToElement(vertex, &pReal);

            Ogre::Vector3 pt(pReal[0], pReal[1], pReal[2]);

			shared_vertices.push_back((orient * (pt * scale)) + position);
		}
		vbuf->unlock();
	}

    // Calculate how many currentSubMesh.mVertices and currentSubMesh.mIndices we're going to need
	for (unsigned short i = 0; i < mEntity->getMesh()->getNumSubMeshes(); ++i)
	{
		currentSubMesh = new SubMeshInformation();
		currentSubMesh->mSubMesh = mEntity->getMesh()->getSubMesh( i );
		Ogre::VertexData* vertex_data = currentSubMesh->mSubMesh->vertexData;
		if (vertex_data) currentSubMesh->mVertex_count = currentSubMesh->mSubMesh->vertexData->vertexCount;
		if (currentSubMesh->mSubMesh->useSharedVertices)
		{
			currentSubMesh->mVertex_count += shared_vertices.size();
		}
		currentSubMesh->mIndex_count = currentSubMesh->mSubMesh->indexData->indexCount;
		currentSubMesh->mVertices = new Ogre::Vector3[currentSubMesh->mVertex_count];
		currentSubMesh->mIndices = new unsigned long[currentSubMesh->mIndex_count];
		currentSubMesh->mIndex = i;

		if (currentSubMesh->mSubMesh->useSharedVertices)
		{
			int inc = 0;
			for (std::vector<Ogre::Vector3>::iterator svi = shared_vertices.begin(); svi != shared_vertices.end(); svi++)
			{
				currentSubMesh->mVertices[inc] = (*svi);
				inc++;
			}
		}
		if (vertex_data)
		{
            const Ogre::VertexElement* posElem =
                vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);

            Ogre::HardwareVertexBufferSharedPtr vbuf =
                vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());

            unsigned char* vertex =
                static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

            // There is _no_ baseVertexPointerToElement() which takes an Ogre::Real or a double
            //  as second argument. So make it float, to avoid trouble when Ogre::Real will
            //  be comiled/typedefed as double:
            //      Ogre::Real* pReal;
            float* pReal;

			for( size_t j = shared_vertices.size(); j < currentSubMesh->mVertex_count; ++j, vertex += vbuf->getVertexSize())
            {
                posElem->baseVertexPointerToElement(vertex, &pReal);

                Ogre::Vector3 pt(pReal[0], pReal[1], pReal[2]);

                currentSubMesh->mVertices[j] = (orient * (pt * scale)) + position;
            }

            vbuf->unlock();
		}


        Ogre::IndexData* index_data = currentSubMesh->mSubMesh->indexData;
        size_t numTris = index_data->indexCount / 3;
        Ogre::HardwareIndexBufferSharedPtr ibuf = index_data->indexBuffer;

        bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);

        unsigned long*  pLong = static_cast<unsigned long*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
        unsigned short* pShort = reinterpret_cast<unsigned short*>(pLong);

        if ( use32bitindexes )
        {
            for ( size_t k = 0; k < numTris*3; ++k)
            {
                currentSubMesh->mIndices[k] = pLong[k];
            }
        }
        else
        {
            for ( size_t k = 0; k < numTris*3; ++k)
            {
                currentSubMesh->mIndices[k] = static_cast<unsigned long>(pShort[k]);
            }
        }

        ibuf->unlock();

		returner.push_back(currentSubMesh);

    }

	return returner;

}

Ogre::SubEntity* EntityMaterialInspector::GetSubEntity(Ogre::Ray ray)
{
	//Ice::Log::Instance().LogMessage("GetSubmesh 1");
	std::vector<SubMeshInformation*> submeshes = GetSubMeshes(mEntity->getParentNode()->_getDerivedPosition(), mEntity->getParentNode()->_getDerivedOrientation(), mEntity->getParentNode()->_getDerivedScale());
	float closest_distance = 99999;
	SubMeshInformation *closest_submesh = NULL;
	for (std::vector<SubMeshInformation*>::iterator mesh = submeshes.begin(); mesh != submeshes.end(); mesh++)
	{
		for (int i = 0; i < static_cast<int>((*mesh)->mIndex_count); i += 3)
		{
			//Ice::Log::Instance().LogMessage("muh");
			std::pair<bool, Ogre::Real> hit = Ogre::Math::intersects(ray, (*mesh)->mVertices[(*mesh)->mIndices[i]],
				(*mesh)->mVertices[(*mesh)->mIndices[i+1]], (*mesh)->mVertices[(*mesh)->mIndices[i+2]], true, false);

			if (hit.first)
			{
				if ( hit.second < closest_distance)
				{
					closest_distance = hit.second;
					closest_submesh = (*mesh);
				}
			}
		}
	}

	Ogre::SubEntity *returner = NULL;
	if (closest_submesh)
	{
		returner = mEntity->getSubEntity(closest_submesh->mIndex);
	}
	//else Ice::Log::Instance().LogMessage("Warning: EntityMaterialInspector::GetSubEntity(Ogre::Ray ray) - return NULL");
	for (std::vector<SubMeshInformation*>::iterator mesh = submeshes.begin(); mesh != submeshes.end(); mesh++)
	{
		delete (*mesh);
	}
	return returner;
}

Ogre::String EntityMaterialInspector::GetMaterialName(Ogre::Ray ray)
{
	Ogre::SubEntity *mesh = GetSubEntity(ray);
	if (mesh) return mesh->getMaterialName();
	return "NORESULT";
}