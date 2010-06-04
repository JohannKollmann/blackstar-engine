

#pragma once

#pragma warning(disable:4482)
#pragma warning(disable:4251)
#pragma warning(disable:4305)		//implicit cast from double to float
#pragma warning(disable:4244)

//#define BOOST_ALL_DYN_LINK

namespace OgrePhysX
{
	class World;
	class Scene;
};
namespace OgreOggSound
{
	class OgreOggSoundManager;
};
class NxControllerManager;

namespace Ice
{
	#define DllExport __declspec(dllexport)

	class Main;
	class MainLoop;
	class SceneManager;
	class LevelMesh;
	class Input;
	class CameraController;
	class Msg;
	class DataMap;
	class MessageSystem;
	class MessageListener;
	class ObjectLevelRayCaster;
	class CameraController;
	class WeatherController;
	class CompositorLoader;
	class ScriptedCollisionCallback;
	class GameObject;
	class ManagedGameObject;
	class GOComponent;
	class GOCViewListener;
	class GOCRigidBody;
	class GOCAI;
	class ScriptedAIState;
	class DayCycle;
	class AIManager;
	class AStarEdge;
	class AStarNode;
	class SoundMaterialTable;
	class Scriptable;
};