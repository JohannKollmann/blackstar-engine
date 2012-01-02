#include <Ogre.h>

class DestructibleMeshSplitter
{
public:
	enum EBoolean_Op
	{
		INSIDE,
		OUTSIDE
	};

	/*
	 * splits mesh into fragments
	 * @param inMesh mesh to split
	 * @param fMaxSize maximum allowable size of fragments
	 * @param fRoughness roughness parameter for split plane
	 * @param fResolution size of one texture unit compared to mesh size
	 * @param strCutMaterial material to assign to cutting plane
	 * @return vector of mesh fragments
	 */
	static std::vector<Ogre::MeshPtr> SplitMesh(Ogre::MeshPtr inMesh,
			float fMaxSize, float fRoughness, float fResolution,
			unsigned int nRecoveryAttempts,
			bool bCutSurface, const Ogre::String& strCutMaterial);

	/*
	 * splits a mesh into two parts (which can contain several distinct parts though)
	 * @param inMesh mesh to split
	 * @param outMesh1 split part 1
	 * @param outMesh2 split part 2
	 * @param bError flag indicating an error of some kind, initialize this to false
	 * all other params @see SplitMesh
	 *
	 */
	static void Split(Ogre::MeshPtr inMesh, Ogre::MeshPtr& outMesh1, Ogre::MeshPtr& outMesh2,
			float fRoughness, float fResolution,
			bool bCutSurface, Ogre::String strCutMaterial, bool& bError,
			Ogre::MeshPtr debugCutPlane=Ogre::MeshPtr(0));

	/*
	 * performs boolean operation on ogre meshes
	 * @param inMesh1 operand 1
	 * @param inMesh1 operand 2
	 * @param op1 operation on first operand
	 * @param op2 operation on second operand
	 * @param bError flag indicating an error of some kind, initialize this to false
	 * @return result of boolean operation
	 */
	static Ogre::MeshPtr BooleanOp(Ogre::MeshPtr inMesh1, Ogre::MeshPtr inMesh2,
			EBoolean_Op op1, EBoolean_Op op2, bool& bError,
			Ogre::MeshPtr debugReTri=Ogre::MeshPtr(0), Ogre::MeshPtr debugCutLine=Ogre::MeshPtr(0));


	static Ogre::MeshPtr loadMesh(Ogre::String strFile);
	static void unloadMesh(Ogre::MeshPtr mesh);
};
