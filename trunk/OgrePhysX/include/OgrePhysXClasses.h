#pragma once

#ifndef WIN32
	#define WIN32
#endif

#define OgrePhysXClass __declspec(dllexport)

#define OGREPHYSX_CANMOVESTATICACTORS 1

#define PX_SUPPORT_VISUAL_DEBUGGER 1	//remote debugger on

//forward declarations
class PxDefaultAllocator;	
class PxPhysics;
class PxScene;
class PxSceneDesc;
class PxCooking;
class PxMaterial;
class PxActor;
class PxGeometry;

namespace OgrePhysX
{
	class World;
	class Scene;
	class LogOutputStream;
	class Cooker;
	class CookerParams;
	class PointRenderable;
	class RenderedActorBinding;
	class Shape;
	class BaseMeshShape;
	class CookedMeshShape;
	class RTMeshShape;
	class RTConvexMeshShape;
	class PrimitiveShape;
	class PlaneShape;
	class BoxShape;
	class SphereShape;
	class CapsuleShape;
	class ContactReportListener;
	class TriggerReportListener;
	class Ragdoll;
}