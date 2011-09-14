
#include "OgrePhysXCooker.h"
#include "OgrePhysXStreams.h"
#include "OgrePhysXWorld.h"
#include "OgrePhysXConvert.h"
#include "UserStream.h"
#include "PxPhysics.h"
#include "PxPhysicsAPI.h"

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

	bool Cooker::hasPxMesh(Ogre::String PxsFile)
	{
		return Ogre::ResourceGroupManager::getSingleton().resourceExists(mOgreResourceGroup, PxsFile);
	}

	PxTriangleMesh* Cooker::loadPxTriangleMeshFromFile(Ogre::String pxsFile)
	{
		if (!hasPxMesh(pxsFile))
		{
			//throw exception
			return 0;
		}
		Ogre::DataStreamPtr ds = Ogre::ResourceGroupManager::getSingleton().openResource(pxsFile);
		return World::getSingleton().getSDK()->createTriangleMesh(OgreReadStream(ds));
	}

	void Cooker::getMeshInfo(Ogre::MeshPtr mesh, CookerParams &params, MeshInfo &outInfo)
	{
		//First, we compute the total number of vertices and indices and init the buffers.
		unsigned int numVertices = 0;
		unsigned int numIndices = 0;

		if (mesh->sharedVertexData) numVertices += mesh->sharedVertexData->vertexCount;
		Ogre::Mesh::SubMeshIterator i = mesh->getSubMeshIterator();
		bool indices32 = true;
		while (i.hasMoreElements())
		{
			Ogre::SubMesh *subMesh = i.getNext();
			if (subMesh->vertexData) numVertices += subMesh->vertexData->vertexCount;
			if (params.mAddBackfaces)
				numIndices += subMesh->indexData->indexCount*2;
			else
				numIndices += subMesh->indexData->indexCount;
				
			//We assume that every submesh uses the same index format
			indices32 = (subMesh->indexData->indexBuffer->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);
		}

		outInfo.vertices.resize(numVertices);
		outInfo.indices.resize(numIndices);
		outInfo.materials.resize(numIndices / 3);

		unsigned int addedVertices = 0;
		unsigned int addedIndices = 0;
		unsigned int addedMaterialIndices = 0;

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
				Ogre::Vector3 vec;
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
					Ogre::Vector3 vec;
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

				PxU32 *pIndices = 0;
				if (indices32)
				{
					pIndices = static_cast<PxU32*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
				}
				else
				{
					PxU16 *pShortIndices = static_cast<PxU16*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
					pIndices = new PxU32[index_data->indexCount];
					for (size_t k = 0; k < index_data->indexCount; k++) pIndices[k] = static_cast<PxU32>(pShortIndices[k]);
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

				//All triangles of a submesh have the same material
				unsigned int numTris = index_data->indexCount / 3;
				if (params.mAddBackfaces) numTris *= 2;
				for (size_t i = addedMaterialIndices; i < addedMaterialIndices+numTris; i++) outInfo.materials[i] = subMesh->getMaterialName();
				addedMaterialIndices += numTris;
			}

			if (vertex_data) index_offset += vertex_data->vertexCount;

		}
	}

	struct OgrePhysXClass OctreeNode
	{
		OctreeNode(){vPos.x=0.0f;vPos.y=0.0f;vPos.z=0.0f;
			aSubNodes[0]=0;aSubNodes[1]=0;aSubNodes[2]=0;aSubNodes[3]=0;aSubNodes[4]=0;aSubNodes[5]=0;aSubNodes[6]=0;aSubNodes[7]=0;}
		Ogre::Vector3 vPos;
		OctreeNode* aSubNodes[8];
		std::list<int> liIndices;
	};
	
	struct OgrePhysXClass STri
	{
		STri(){i1=-1;i2=-1;i3=-1;}
		STri(int iIndex1, int iIndex2, int iIndex3, Ogre::String material, bool bSort=true)
		{
			if(!bSort)
			{
				i1=iIndex1;
				i2=iIndex2;
				i3=iIndex3;
				return;
			}
			//rotate indices
			if(iIndex2<iIndex1)
			{
				if(iIndex3<iIndex2)
				{//index 3 is the smallest
					i1=iIndex3;
					i2=iIndex1;
					i3=iIndex2;
				}
				else
				{
					i1=iIndex2;
					i2=iIndex3;
					i3=iIndex1;
				}
			}
			else
			{
				i1=iIndex1;
				i2=iIndex2;
				i3=iIndex3;
			}
		}
		bool operator !=(STri& op){if(op.i1!=i1 || op.i2!=i2 || op.i3!=i3) return true; return false;}
		bool operator <(STri& op)
		{
			if(op.i1!=i1)
				return i1<op.i1;
			if(op.i2!=i2)
				return i2<op.i2;
			return i3<op.i3;
		}
		int i1,i2,i3;
		Ogre::String mat;
	};

	//returns current vertex count
	int OgrePhysXClass ExtractOctree(OctreeNode* pNode, int iVertexCount, int* aiIndexTable, Ogre::Vector3* aNewVertices)
	{
		for(std::list<int>::const_iterator it=pNode->liIndices.begin();
			it!=pNode->liIndices.end(); it++)
			aiIndexTable[*it]=iVertexCount;
		aNewVertices[iVertexCount++]=pNode->vPos;
		for(int iSubNode=0; iSubNode<8; iSubNode++)
			if(pNode->aSubNodes[iSubNode])
			{
				iVertexCount=ExtractOctree(pNode->aSubNodes[iSubNode], iVertexCount, aiIndexTable, aNewVertices);
				delete pNode->aSubNodes[iSubNode];
				pNode->aSubNodes[iSubNode] = nullptr;
			}
		return iVertexCount;
	}


#define IS_IN_BOX(v1,v2,d) ((v1.x<=v2.x+d) && (v1.x>=v2.x-d) && (v1.y<=v2.y+d) && (v1.y>=v2.y-d) && (v1.z<=v2.z+d) && (v1.z>=v2.z-d))

#define EIGHTH_SPACE_INDEX(v1,v2) (((v1.x>v2.x)?4:0)+((v1.y>v2.y)?2:0)+((v1.z>v2.z)?1:0))

	void Cooker::mergeVertices(MeshInfo &meshInfo, float fMergeDist)
	{
		//const float fMergeDist=1e-3f;

		OctreeNode root;
		root.vPos=meshInfo.vertices[0];
		int iVertex=0;
		int numAdded = 0;
		
		for(;iVertex<(int)meshInfo.vertices.size(); iVertex++)
		{
			OctreeNode* pCurrNode=&root;
			while(true)
			{
				if(IS_IN_BOX(meshInfo.vertices[iVertex], pCurrNode->vPos, fMergeDist))
				{
					pCurrNode->liIndices.push_back(iVertex);
					break;
				}
				else
				{//vertex is not in merge distance to this node
					int iSubNode=EIGHTH_SPACE_INDEX(pCurrNode->vPos, meshInfo.vertices[iVertex]);
					if(pCurrNode->aSubNodes[iSubNode])
						//proceed deeper into the tree
						pCurrNode=pCurrNode->aSubNodes[iSubNode];
					else
					{//there is no branch so make one
						pCurrNode->aSubNodes[iSubNode]=new OctreeNode;
						pCurrNode=pCurrNode->aSubNodes[iSubNode];
						pCurrNode->vPos=meshInfo.vertices[iVertex];
						numAdded++;
					}
				}//pCurrNode is now one level lower in the tree
			}
		}
		int* aiIndexTable=new int[meshInfo.vertices.size()];		//maps old indices to new 		
		Ogre::Vector3* aNewVertices=new Ogre::Vector3[meshInfo.vertices.size()];
		//extract indextable and vertex list
		int nNewVertices=ExtractOctree(&root, 0, aiIndexTable, aNewVertices);
		for(int iIndex=0; iIndex<(int)meshInfo.indices.size(); ++iIndex)
		{
			assert(meshInfo.indices[iIndex] < meshInfo.indices.size());
			assert(meshInfo.indices[iIndex] >= 0);
			meshInfo.indices[iIndex] = aiIndexTable[meshInfo.indices[iIndex]];
		}
		
		meshInfo.vertices.resize(nNewVertices);
		for(iVertex=0; iVertex<nNewVertices; iVertex++)
			meshInfo.vertices[iVertex]=aNewVertices[iVertex];
		
		delete aiIndexTable;
		delete aNewVertices;
		
		//search for duplicated and degenerate tris
		std::vector<STri> vTris;
		vTris.resize(meshInfo.indices.size() / 3);
		int nTrisCopied=0;
		int iTri=0;
		for(; iTri<(int)meshInfo.indices.size() / 3; iTri++)
		{//check if this tri is degenerate
			int index1=meshInfo.indices[iTri*3+0],
				index2=meshInfo.indices[iTri*3+1],
				index3=meshInfo.indices[iTri*3+2];
			if(index1==index2 || index3==index2 || index1==index3)
				//degenerate tri: two or more vertices are the same
				continue;
			vTris[nTrisCopied++]=STri(index1,index2,index3, meshInfo.materials[iTri]);
		}
		vTris.resize(nTrisCopied);
		std::sort(vTris.begin(), vTris.end());//sort tris to find duplicates easily
		nTrisCopied=0;
		STri lastTri;
		for(iTri=0; iTri<(int)vTris.size(); iTri++)
		{
			if(lastTri!=vTris[iTri])
			{
				meshInfo.indices[nTrisCopied*3+0]=vTris[iTri].i1;
				meshInfo.indices[nTrisCopied*3+1]=vTris[iTri].i2;
				meshInfo.indices[nTrisCopied*3+2]=vTris[iTri].i3;
				meshInfo.materials[nTrisCopied]=vTris[iTri].mat;
				lastTri=vTris[iTri];
				nTrisCopied++;
			}
		}
		meshInfo.materials.resize(nTrisCopied);
		meshInfo.indices.resize(nTrisCopied*3);
	}

	//function to generate ccd mesh
	void Cooker::insetMesh(MeshInfo &meshInfo, float fAmount)
	{
		//STri* tris= new STri[meshInfo.numTriangles];
		std::vector<STri> vTris;
		vTris.resize(meshInfo.indices.size()/3);//create rotated tris
		int iTri=0;
		for(; iTri<(int)meshInfo.indices.size()/3; iTri++)
		{
			vTris[iTri*3]=STri(meshInfo.indices[iTri*3], meshInfo.indices[iTri*3+1], meshInfo.indices[iTri*3+2], 0, false);//no mat indices this time!
			vTris[iTri*3+1]=STri(meshInfo.indices[iTri*3+2], meshInfo.indices[iTri*3], meshInfo.indices[iTri*3+1], 0, false);
			vTris[iTri*3+2]=STri(meshInfo.indices[iTri*3+1], meshInfo.indices[iTri*3+2], meshInfo.indices[iTri*3], 0, false);
		}
		std::sort(vTris.begin(), vTris.end());
		int iLastIndex=-1;
		int nVertices;
		Ogre::Vector3 vAccNomals;
		Ogre::Vector3 vPos;
		std::vector<Ogre::Vector3> vNewVertices;
		vNewVertices.resize(meshInfo.vertices.size());
		for(iTri=0; iTri<(int)vTris.size(); iTri++)
		{
			if(vTris[iTri].i1!=iLastIndex)
			{
				if(iLastIndex!=-1)
				{
					vAccNomals.normalise();
					vNewVertices[iLastIndex]=meshInfo.vertices[iLastIndex]-fAmount*vAccNomals;
				}
				nVertices=0;
				vAccNomals=Ogre::Vector3(0,0,0);
				iLastIndex=vTris[iTri].i1;
				vPos=meshInfo.vertices[iLastIndex];
			}
			Ogre::Vector3 v=(meshInfo.vertices[vTris[iTri].i2]-vPos).crossProduct(meshInfo.vertices[vTris[iTri].i3]-vPos);
			v.normalise();
			vAccNomals+=v;
			nVertices++;
		}
		vAccNomals.normalise();
		vNewVertices[iLastIndex]=meshInfo.vertices[iLastIndex]+fAmount*vAccNomals;
		for(unsigned int iVertex=0; iVertex<meshInfo.vertices.size(); iVertex++)
			meshInfo.vertices[iVertex]=vNewVertices[iVertex];
	}

	void Cooker::cutMesh(MeshInfo &outInfo, PxVec3 vPlanePos, PxVec3 vPlaneDir)
	{
		/*
		cutting an arbitrary closed triangle mesh

		test all lines for intersection with plane -> one vert front, one back

		generate verts on intersecting lines, reset indices from beyond cutting plane to newly generated ones

		regenerate tris: if only one vertex of tri is beyond cutting plane generate quad by using both indices on 

		the near side and one newly generated, then generate new indices for the second tri using two newly 

		generated vertices and the first of the last tri

		the cutted side:
		find loops. all vertices generated by the cut are on one (!) loop. one should be able to go from one vertex 
		to another by looking up in the index buffer to which vertex they are connected. generate look up table only 
		for indices on the cutting plane. these should be added while regenerating the mesh towards cutting plane.

		the loops may be nested but must not intersect (this would mean that the mesh is not correct). sort the 
		loops into a multitree.

		check for parent loop by finding uneven numbers of intersections with a loop (even number means child).
		do this by setting up a plane with the sector as normal and traversing all loops checking for intersection.
		all loops identified as parent don't have to be checked later.
		
		the resulting forest has to be sorted by a nlogn algorithm

		auto-triangulation of an outer loop with n loops inside.

		start with an outside edge.

		cast plane along connection lines and check for collision with other loops.

		if the algo has found a link between two loops try to do a zig-zag by making quads towards the next two vertices along the direction along the loops.
		switch direction if the first such connection fails.
		if zig-zag doesn't work try fanning (simply decrement the start index of the zig zag)
		when everything fails search for another loop to continue with (there will always be one)

		check for incomplete mesh: walk along loops checking wether two consecutive indices have no common tri

		regenerate all buffers
		*/
	}

	void Cooker::cookPxTriangleMesh(Ogre::MeshPtr mesh, PxStream& outputStream, CookerParams &params)		//Ogre::Vector3 scale, std::map<Ogre::String, PxMaterialIndex> &materialBindings)
	{
		MeshInfo meshInfo;
		getMeshInfo(mesh, params, meshInfo);
		mergeVertices(meshInfo);

		PxTriangleMeshDesc desc;
		desc.setToDefault();

		desc.points.count = meshInfo.vertices.size();
		desc.points.stride = 12;
		float *fVertices = new float[meshInfo.vertices.size()*3];
		for (unsigned int i = 0; i < meshInfo.vertices.size(); ++i)
		{
			fVertices[i*3] = meshInfo.vertices[i].x;
			fVertices[i*3+1] = meshInfo.vertices[i].y;
			fVertices[i*3+2] = meshInfo.vertices[i].z;
		}
		desc.points.data = fVertices;
		
		desc.triangles.count = meshInfo.indices.size() / 3;
		desc.triangles.stride = 12;
		PxU32 *iIndices = new PxU32[meshInfo.indices.size()];
		for (unsigned int i = 0; i < meshInfo.indices.size(); ++i)
			iIndices[i] = meshInfo.indices[i];
		desc.triangles.data = iIndices;

		PxMaterialTableIndex *materialIndices = nullptr;
		if (!params.mMaterialBindings.empty())
		{
			desc.materialIndices.stride = sizeof(PxMaterialTableIndex);
			materialIndices = new PxMaterialTableIndex[meshInfo.indices.size() / 3];
			for (unsigned int i = 0; i < meshInfo.indices.size() / 3; ++i)
			{
				auto matIndex = params.mMaterialBindings.find(meshInfo.materials[i]);
				if (matIndex == params.mMaterialBindings.end())
				{
					Ogre::LogManager::getSingleton().logMessage("[OgrePhysX] Could not find material index for material: " + meshInfo.materials[i]);
					materialIndices[i] = 0;
				}
				else materialIndices[i] = matIndex->second;
			}	
		}
		desc.materialIndices.data = materialIndices;

		//dump the fucking buffers!
		/*for (unsigned int i = 0; i < outInfo.numTriangles*3; i+=3)
			Ogre::LogManager::getSingleton().logMessage(Ogre::StringConverter::toString(meshInfo.indices[i]) + " " + Ogre::StringConverter::toString(all_indices[i+1]) + " " + Ogre::StringConverter::toString(all_indices[i+2]));

		for (unsigned int i = 0; i < outInfo.numVertices; i++)
			Ogre::LogManager::getSingleton().logMessage(Ogre::StringConverter::toString(Convert::toOgre(meshInfo.vertices[i])));*/

		World::getSingleton().getCookingInterface()->cookTriangleMesh(desc, outputStream);

		delete fVertices;
		delete iIndices;
		if (materialIndices) delete materialIndices;

	}

	void Cooker::cookPxConvexMesh(Ogre::MeshPtr mesh, PxStream& outputStream, CookerParams &params)		//Ogre::Vector3 scale, std::map<Ogre::String, PxMaterialIndex> &materialBindings)
	{
		MeshInfo meshInfo;
		getMeshInfo(mesh, params, meshInfo);

		PxConvexMeshDesc desc;
		desc.points.count = meshInfo.vertices.size();
		desc.points.stride = 12;
		float *fVertices = new float[meshInfo.vertices.size()*3];
		for (unsigned int i = 0; i < meshInfo.vertices.size(); ++i)
		{
			fVertices[i*3] = meshInfo.vertices[i].x;
			fVertices[i*3+1] = meshInfo.vertices[i].y;
			fVertices[i*3+2] = meshInfo.vertices[i].z;
		}
		desc.points.data = fVertices;
		
		desc.triangles.count = meshInfo.indices.size() / 3;
		desc.triangles.stride = 12;
		int *iIndices = new int[meshInfo.indices.size()];
		for (unsigned int i = 0; i < meshInfo.indices.size(); ++i)
			iIndices[i] = meshInfo.indices[i];
		desc.triangles.data = iIndices;

		World::getSingleton().getCookingInterface()->cookConvexMesh(desc, outputStream);

		delete fVertices;
		delete iIndices;
	}

	void Cooker::cookPxTriangleMeshToFile(Ogre::MeshPtr mesh, Ogre::String pxsOutputFile, CookerParams &params)
	{
		cookPxTriangleMesh(mesh, PxToolkit::UserStream(pxsOutputFile.c_str(), true), params);
	}

	PxTriangleMesh* Cooker::createPxTriangleMesh(Ogre::MeshPtr mesh, CookerParams &params)	
	{
		PxToolkit::MemoryWriteBuffer stream;
		cookPxTriangleMesh(mesh, stream, params);
		if (stream.data == nullptr) return nullptr;
		return World::getSingleton().getSDK()->createTriangleMesh(PxToolkit::MemoryReadBuffer(stream.data));
	}

	PxConvexMesh* Cooker::createPxConvexMesh(Ogre::MeshPtr mesh, CookerParams &params)	
	{
		PxToolkit::MemoryWriteBuffer stream;
		cookPxConvexMesh(mesh, stream, params);
		return World::getSingleton().getSDK()->createConvexMesh(PxToolkit::MemoryReadBuffer(stream.data));
	}

	Cooker& Cooker::getSingleton()
	{
		static Cooker instance;
		return instance;
	}

}
