
#include "OgrePhysXCooker.h"
#include "OgrePhysXStreams.h"
#include "OgrePhysXWorld.h"
#include "NXU_Streaming.h"
#include "NXU_Streaming.cpp"
#include "NXU_File.cpp"
#include "OgrePhysXConvert.h"

namespace OgrePhysX
{

	Cooker::Cooker(void)
	{
		mOgreResourceGroup = "General";
	}

	Cooker::~Cooker(void)
	{
	}

	void Cooker::setOgreResourceGroup(Ogre::String group)
	{
		mOgreResourceGroup = group;
	}

	bool Cooker::hasNxMesh(Ogre::String nxsFile)
	{
		return Ogre::ResourceGroupManager::getSingleton().resourceExists(mOgreResourceGroup, nxsFile);
	}

	NxTriangleMesh* Cooker::loadNxMeshFromFile(Ogre::String nxsFile)
	{
		if (!hasNxMesh(nxsFile))
		{
			//throw exception
			return 0;
		}
		Ogre::DataStreamPtr ds = Ogre::ResourceGroupManager::getSingleton().openResource(nxsFile);
		return World::getSingleton().getSDK()->createTriangleMesh(OgreReadStream(ds));
	}


	void Cooker::cookNxMesh(Ogre::MeshPtr mesh, NxStream& outputStream, CookerParams &params)		//Ogre::Vector3 scale, std::map<Ogre::String, NxMaterialIndex> &materialBindings)
	{
		// Build the triangle mesh.
		NxTriangleMeshDesc meshDesc;
		meshDesc.numVertices                = 0;
		meshDesc.numTriangles               = 0;
		meshDesc.materialIndexStride		= sizeof(NxMaterialIndex);
		meshDesc.pointStrideBytes           = sizeof(NxVec3);
		meshDesc.triangleStrideBytes        = 3 * sizeof(NxU32);

		//First, we compute the total number of vertices and indices and create the buffers.
		if (mesh->sharedVertexData) meshDesc.numVertices += mesh->sharedVertexData->vertexCount;
		Ogre::Mesh::SubMeshIterator i = mesh->getSubMeshIterator();
		bool indices32 = true;
		while (i.hasMoreElements())
		{
			Ogre::SubMesh *subMesh = i.getNext();
			if (subMesh->vertexData) meshDesc.numVertices += subMesh->vertexData->vertexCount;
			if (params.mAddBackfaces)
				meshDesc.numTriangles += ((subMesh->indexData->indexCount*2) / 3);
			else
				meshDesc.numTriangles += (subMesh->indexData->indexCount / 3);
				
			//We assume that every submesh uses the same index format
			indices32 = (subMesh->indexData->indexBuffer->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);
		}

		NxArray<NxVec3> all_vertices;
		all_vertices.resize(meshDesc.numVertices);
		NxArray<NxU32> all_indices;
		all_indices.resize(meshDesc.numTriangles * 3);
		NxArray<NxMaterialIndex> all_materialIndices;
		all_materialIndices.resize(meshDesc.numTriangles);

		size_t addedVertices = 0;
		size_t addedIndices = 0;
		size_t addedMaterialIndices = 0;

		/*
		Read shared vertices
		*/
		unsigned int shared_index_offset = 0;
		Ogre::VertexData *shared_vertex_data = mesh->sharedVertexData;
		if (shared_vertex_data)
		{
			const Ogre::VertexElement* posElem =
				shared_vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
			Ogre::HardwareVertexBufferSharedPtr vbuf =
				shared_vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());

			shared_index_offset = shared_vertex_data->vertexCount;

			unsigned char* pVertices = static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
			//size_t added = all_vertices.size();
			//all_vertices.resize(all_vertices.size()+shared_vertex_data->vertexCount);
			Ogre::Real* pReal;
			for (size_t i = addedVertices; i < shared_vertex_data->vertexCount; i++)
			{
				posElem->baseVertexPointerToElement(pVertices, &pReal);
				NxVec3 vec;
				vec.x = (*pReal++) * params.mScale.x;
				vec.y = (*pReal++) * params.mScale.y;
				vec.z = (*pReal++) * params.mScale.z;
				all_vertices[i] = vec;
				pVertices += vbuf->getVertexSize();
			}
			vbuf->unlock();
			addedVertices += shared_vertex_data->vertexCount;

		}

		unsigned int index_offset = 0;

		/*
		Read submeshes
		*/
		i = mesh->getSubMeshIterator();
		while (i.hasMoreElements())
		{
			Ogre::SubMesh *subMesh = i.getNext();

			//Read vertex data
			Ogre::VertexData *vertex_data = subMesh->vertexData;
			if (vertex_data)
			{
				const Ogre::VertexElement* posElem =
					vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
				Ogre::HardwareVertexBufferSharedPtr vbuf =
					vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());

				unsigned char* pVertices = static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
				//size_t added = all_vertices.size();
				//all_vertices.resize(all_vertices.size()+vertex_data->vertexCount);
				Ogre::Real* pReal;
				for (size_t i = addedVertices; i < addedVertices+vertex_data->vertexCount; i++)
				{
					posElem->baseVertexPointerToElement(pVertices, &pReal);
					NxVec3 vec;
					vec.x = (*pReal++) * params.mScale.x;
					vec.y = (*pReal++) * params.mScale.y;
					vec.z = (*pReal++) * params.mScale.z;
					all_vertices[i] = vec;
					pVertices += vbuf->getVertexSize();
				}
				addedVertices += vertex_data->vertexCount;

				vbuf->unlock();
			}

			//Read index data
			Ogre::IndexData *index_data = subMesh->indexData;
			if (index_data)
			{
				Ogre::HardwareIndexBufferSharedPtr ibuf = index_data->indexBuffer;

				NxU32 *pIndices = 0;
				if (indices32)
				{
					pIndices = static_cast<NxU32*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
				}
				else
				{
					NxU16 *pShortIndices = static_cast<NxU16*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
					pIndices = new NxU32[index_data->indexCount];
					for (size_t k = 0; k < index_data->indexCount; k++) pIndices[k] = static_cast<NxU32>(pShortIndices[k]);
				}
				/*size_t added = all_indices.size();
				if (params.mAddBackfaces) 	all_indices.resize(all_indices.size()+(index_data->indexCount*2));
				else 						all_indices.resize(all_indices.size()+index_data->indexCount);*/
				unsigned int bufferIndex = 0;
				if (params.mAddBackfaces)
				{
					size_t numTris = index_data->indexCount / 3;
					size_t i = addedIndices;
					for (unsigned int x = 0; x < numTris; x++)
					{
						if (subMesh->useSharedVertices)
						{
							if (pIndices[bufferIndex] > shared_index_offset) all_indices[i] = pIndices[bufferIndex] + index_offset;
							else all_indices[i] = pIndices[bufferIndex];
							bufferIndex++;
							if (pIndices[bufferIndex] > shared_index_offset) all_indices[i+1] = pIndices[bufferIndex] + index_offset;
							else all_indices[i+1] = pIndices[bufferIndex];
							bufferIndex++;
							if (pIndices[bufferIndex] > shared_index_offset) all_indices[i+2] = pIndices[bufferIndex] + index_offset;
							else all_indices[i+2] = pIndices[bufferIndex];
							bufferIndex++;
						}
						else
						{
							all_indices[i] = pIndices[bufferIndex] + index_offset;
							bufferIndex++;
							all_indices[i+1] = pIndices[bufferIndex] + index_offset;
							bufferIndex++;
							all_indices[i+2] = pIndices[bufferIndex] + index_offset;
							bufferIndex++;
						}
						all_indices[i+3] = all_indices[i+2];
						all_indices[i+4] = all_indices[i+1];
						all_indices[i+5] = all_indices[i];
						i += 6;
					}
					addedIndices += index_data->indexCount*2;
				}
				else
				{
					for (size_t i = addedIndices; i < addedIndices+index_data->indexCount; i++)
					{
						if (subMesh->useSharedVertices)
						{
							if (pIndices[bufferIndex] > shared_index_offset) all_indices[i] = pIndices[bufferIndex] + index_offset;
							else all_indices[i] = pIndices[bufferIndex];
						}
						else all_indices[i] = pIndices[bufferIndex] + index_offset;
						bufferIndex++;
					}
					addedIndices += index_data->indexCount;
				}
				if (!indices32) delete pIndices;

				ibuf->unlock();

				//All triangles of a submesh have the same material.
				unsigned int numTris = index_data->indexCount / 3;
				if (params.mAddBackfaces) numTris *= 2;
				//added = all_materialIndices.size();
				//all_materialIndices.resize(all_materialIndices.size()+numTris);
				std::map<Ogre::String, NxMaterialIndex>::iterator i = params.mMaterialBindings.find(subMesh->getMaterialName());
				NxMaterialIndex physXMat = (i != params.mMaterialBindings.end()) ? i->second : 0;
				for (size_t i = addedMaterialIndices; i < addedMaterialIndices+numTris; i++) all_materialIndices[i] = physXMat;
				addedMaterialIndices += numTris;
			}

			if (vertex_data) index_offset += vertex_data->vertexCount;

		}
		
		//dump the fucking buffers!
		/*for (unsigned int i = 0; i < meshDesc.numTriangles*3; i+=3)
			Ogre::LogManager::getSingleton().logMessage(Ogre::StringConverter::toString(all_indices[i]) + " " + Ogre::StringConverter::toString(all_indices[i+1]) + " " + Ogre::StringConverter::toString(all_indices[i+2]));

		for (unsigned int i = 0; i < meshDesc.numVertices; i++)
			Ogre::LogManager::getSingleton().logMessage(Ogre::StringConverter::toString(Convert::toOgre(all_vertices[i])));*/

		meshDesc.points = &all_vertices[0].x;
		meshDesc.triangles = &all_indices[0];
		meshDesc.materialIndices = &all_materialIndices[0];
		meshDesc.flags = 0;

		World::getSingleton().getCookingInterface()->NxCookTriangleMesh(meshDesc, outputStream);
	}

	void Cooker::cookNxMeshToFile(Ogre::MeshPtr mesh, Ogre::String nxsOutputFile, CookerParams &params)	//Ogre::Vector3 scale, std::map<Ogre::String, NxMaterialIndex> &materialBindings)
	{
		cookNxMesh(mesh, NXU::UserStream(nxsOutputFile.c_str(), false), params);		//scale, materialBindings);
	}

	NxTriangleMesh* Cooker::getNxMesh(Ogre::MeshPtr mesh, CookerParams &params)		//Ogre::Vector3 scale, std::map<Ogre::String, NxMaterialIndex> &materialBindings)
	{
		NXU::MemoryWriteBuffer stream;
		cookNxMesh(mesh, stream, params);	//scale, materialBindings);
		return World::getSingleton().getSDK()->createTriangleMesh(NXU::MemoryReadBuffer(stream.data));
	}

	Cooker& Cooker::getSingleton()
	{
		static Cooker instance;
		return instance;
	}

}
