

#pragma once

#pragma warning(disable:4482)		//"nonstandard expression used in qualified name"
#pragma warning(disable:4305)		//implicit cast from double to float
#pragma warning(disable:4244)		//cast from double to const float
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

#define STRINGIFY(param) #param

#define ICE_NOASSERRTS 0

#ifdef _DEBUG
	#define IceAssert(expression) \
	if (!expression) \
	{ \
		Ogre::LogManager::getSingleton().logMessage(Ogre::String("Assertion failed: ") + #expression +  Ogre::String(", file ") + __FILE__ +  Ogre::String(", line ") + Ogre::StringConverter::toString(__LINE__)); \
		DebugBreak();	\
	}
#else
	#define IceAssert(expression) \
	if (!expression) \
	{ \
		Ogre::LogManager::getSingleton().logMessage("Assertion failed: " ## #expression ## ", file " ## STRINGIFY(__FILE__) ## ", line " ## STRINGIFY(__LINE__)); \
	}
#endif

#if ICE_NOASSERRTS
	#undef IceAssert
	#define IceAssert(expression)
#endif

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
	class GOCSimpleCameraController;
	class GOCCameraController;
};