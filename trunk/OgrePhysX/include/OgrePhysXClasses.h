/*
This source file is part of OgrePhysX.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

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