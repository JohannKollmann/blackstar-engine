
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

	NxTriangleMesh* Cooker::loadNxTriangleMeshFromFile(Ogre::String nxsFile)
	{
		if (!hasNxMesh(nxsFile))
		{
			//throw exception
			return 0;
		}
		Ogre::DataStreamPtr ds = Ogre::ResourceGroupManager::getSingleton().openResource(nxsFile);
		return World::getSingleton().getSDK()->createTriangleMesh(OgreReadStream(ds));
	}

	void Cooker::getMeshInfo(Ogre::MeshPtr mesh, CookerParams &params, MeshInfo &outInfo)
	{
		outInfo.numVertices = 0;
		outInfo.numTriangles = 0;

		//First, we compute the total number of vertices and indices and create the buffers.
		if (mesh->sharedVertexData) outInfo.numVertices += mesh->sharedVertexData->vertexCount;
		Ogre::Mesh::SubMeshIterator i = mesh->getSubMeshIterator();
		bool indices32 = true;
		while (i.hasMoreElements())
		{
			Ogre::SubMesh *subMesh = i.getNext();
			if (subMesh->vertexData) outInfo.numVertices += subMesh->vertexData->vertexCount;
			if (params.mAddBackfaces)
				outInfo.numTriangles += ((subMesh->indexData->indexCount*2) / 3);
			else
				outInfo.numTriangles += (subMesh->indexData->indexCount / 3);
				
			//We assume that every submesh uses the same index format
			indices32 = (subMesh->indexData->indexBuffer->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);
		}

		outInfo.vertices.resize(outInfo.numVertices);
		outInfo.indices.resize(outInfo.numTriangles * 3);
		outInfo.materialIndices.resize(outInfo.numTriangles);

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
			Ogre::Real* pReal;
			for (size_t i = addedVertices; i < shared_vertex_data->vertexCount; i++)
			{
				posElem->baseVertexPointerToElement(pVertices, &pReal);
				NxVec3 vec;
				vec.x = (*pReal++) * params.mScale.x;
				vec.y = (*pReal++) * params.mScale.y;
				vec.z = (*pReal++) * params.mScale.z;
				outInfo.vertices[i] = vec;
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
				Ogre::Real* pReal;
				for (size_t i = addedVertices; i < addedVertices+vertex_data->vertexCount; i++)
				{
					posElem->baseVertexPointerToElement(pVertices, &pReal);
					NxVec3 vec;
					vec.x = (*pReal++) * params.mScale.x;
					vec.y = (*pReal++) * params.mScale.y;
					vec.z = (*pReal++) * params.mScale.z;
					outInfo.vertices[i] = vec;
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
				unsigned int bufferIndex = 0;
				if (params.mAddBackfaces)
				{
					size_t numTris = index_data->indexCount / 3;
					size_t i = addedIndices;
					for (unsigned int x = 0; x < numTris; x++)
					{
						if (subMesh->useSharedVertices)
						{
							if (pIndices[bufferIndex] > shared_index_offset) outInfo.indices[i] = pIndices[bufferIndex] + index_offset;
							else outInfo.indices[i] = pIndices[bufferIndex];
							bufferIndex++;
							if (pIndices[bufferIndex] > shared_index_offset) outInfo.indices[i+1] = pIndices[bufferIndex] + index_offset;
							else outInfo.indices[i+1] = pIndices[bufferIndex];
							bufferIndex++;
							if (pIndices[bufferIndex] > shared_index_offset) outInfo.indices[i+2] = pIndices[bufferIndex] + index_offset;
							else outInfo.indices[i+2] = pIndices[bufferIndex];
							bufferIndex++;
						}
						else
						{
							outInfo.indices[i] = pIndices[bufferIndex] + index_offset;
							bufferIndex++;
							outInfo.indices[i+1] = pIndices[bufferIndex] + index_offset;
							bufferIndex++;
							outInfo.indices[i+2] = pIndices[bufferIndex] + index_offset;
							bufferIndex++;
						}
						outInfo.indices[i+3] = outInfo.indices[i+2];
						outInfo.indices[i+4] = outInfo.indices[i+1];
						outInfo.indices[i+5] = outInfo.indices[i];
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
							if (pIndices[bufferIndex] > shared_index_offset) outInfo.indices[i] = pIndices[bufferIndex] + index_offset;
							else outInfo.indices[i] = pIndices[bufferIndex];
						}
						else outInfo.indices[i] = pIndices[bufferIndex] + index_offset;
						bufferIndex++;
					}
					addedIndices += index_data->indexCount;
				}
				if (!indices32) delete pIndices;

				ibuf->unlock();

				//All triangles of a submesh have the same material.
				unsigned int numTris = index_data->indexCount / 3;
				if (params.mAddBackfaces) numTris *= 2;
				std::map<Ogre::String, NxMaterialIndex>::iterator i = params.mMaterialBindings.find(subMesh->getMaterialName());
				NxMaterialIndex physXMat = (i != params.mMaterialBindings.end()) ? i->second : 0;
				for (size_t i = addedMaterialIndices; i < addedMaterialIndices+numTris; i++) outInfo.materialIndices[i] = physXMat;
				addedMaterialIndices += numTris;
			}

			if (vertex_data) index_offset += vertex_data->vertexCount;

		}

		mergeVertices(outInfo);
	}

	void Cooker::mergeVertices(MeshInfo &meshInfo)
	{
		/* insert merge code here */
	}


	void Cooker::cookNxTriangleMesh(Ogre::MeshPtr mesh, NxStream& outputStream, CookerParams &params)		//Ogre::Vector3 scale, std::map<Ogre::String, NxMaterialIndex> &materialBindings)
	{
		MeshInfo meshInfo;
		getMeshInfo(mesh, params, meshInfo);

		// Build the triangle mesh.
		NxTriangleMeshDesc meshDesc;
		meshDesc.numVertices                = meshInfo.numVertices;
		meshDesc.numTriangles               = meshInfo.numTriangles;
		meshDesc.materialIndexStride		= sizeof(NxMaterialIndex);
		meshDesc.pointStrideBytes           = sizeof(NxVec3);
		meshDesc.triangleStrideBytes        = 3 * sizeof(NxU32);

		meshDesc.points = &meshInfo.vertices[0].x;
		meshDesc.triangles = &meshInfo.indices[0];
		meshDesc.materialIndices = &meshInfo.materialIndices[0];
		meshDesc.flags = 0;

		//dump the fucking buffers!
		/*for (unsigned int i = 0; i < outInfo.numTriangles*3; i+=3)
			Ogre::LogManager::getSingleton().logMessage(Ogre::StringConverter::toString(meshInfo.indices[i]) + " " + Ogre::StringConverter::toString(all_indices[i+1]) + " " + Ogre::StringConverter::toString(all_indices[i+2]));

		for (unsigned int i = 0; i < outInfo.numVertices; i++)
			Ogre::LogManager::getSingleton().logMessage(Ogre::StringConverter::toString(Convert::toOgre(meshInfo.vertices[i])));*/

		World::getSingleton().getCookingInterface()->NxCookTriangleMesh(meshDesc, outputStream);
	}

	void Cooker::cookNxConvexMesh(Ogre::MeshPtr mesh, NxStream& outputStream, CookerParams &params)		//Ogre::Vector3 scale, std::map<Ogre::String, NxMaterialIndex> &materialBindings)
	{
		MeshInfo meshInfo;
		getMeshInfo(mesh, params, meshInfo);

		// Build the triangle mesh.
		NxConvexMeshDesc convexDesc;
		convexDesc.numVertices = meshInfo.numVertices;
		convexDesc.pointStrideBytes = sizeof(NxVec3);
		convexDesc.points = &meshInfo.vertices[0].x;
		convexDesc.flags = NX_CF_COMPUTE_CONVEX;

		World::getSingleton().getCookingInterface()->NxCookConvexMesh(convexDesc, outputStream);
	}

	void Cooker::cookNxTriangleMeshToFile(Ogre::MeshPtr mesh, Ogre::String nxsOutputFile, CookerParams &params)
	{
		cookNxTriangleMesh(mesh, NXU::UserStream(nxsOutputFile.c_str(), false), params);
	}

	NxTriangleMesh* Cooker::createNxTriangleMesh(Ogre::MeshPtr mesh, CookerParams &params)	
	{
		NXU::MemoryWriteBuffer stream;
		cookNxTriangleMesh(mesh, stream, params);
		return World::getSingleton().getSDK()->createTriangleMesh(NXU::MemoryReadBuffer(stream.data));
	}

	NxConvexMesh* Cooker::createNxConvexMesh(Ogre::MeshPtr mesh, CookerParams &params)	
	{
		NXU::MemoryWriteBuffer stream;
		cookNxConvexMesh(mesh, stream, params);
		return World::getSingleton().getSDK()->createConvexMesh(NXU::MemoryReadBuffer(stream.data));
	}

	Cooker& Cooker::getSingleton()
	{
		static Cooker instance;
		return instance;
	}

}
