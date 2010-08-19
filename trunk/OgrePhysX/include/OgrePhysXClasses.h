#pragma once

#ifndef WIN32
	#define WIN32
#endif

#define OgrePhysXClass __declspec(dllexport)

#define OGREPHYSX_CANMOVESTATICACTORS 1

namespace OgrePhysX
{
	class Convert;
	class World;
	class Scene;
	class Cooker;
	class CookerParams;
	class PointRenderable;
	class Actor;
	class RenderedActor;
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