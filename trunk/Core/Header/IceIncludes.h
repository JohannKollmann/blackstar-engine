

#pragma once

#pragma warning(disable:4482)
#pragma warning(disable:4251)

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
	class GOComponent;
	class ComponentSection;
	struct ObjectMsg;
	class GOCViewListener;
	class GOCAI;
	class ScriptedAIState;
	class DayCycle;
	class AIManager;
	class AStarEdge;
	class AStarNode;
	class TriangleNode;
};