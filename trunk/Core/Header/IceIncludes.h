

#pragma once

#pragma warning(disable:4482)		//"nonstandard expression used in qualified name"
#pragma warning(disable:4305)		//implicit cast from double to float
#pragma warning(disable:4244)		//cast from double to const float
#pragma warning(disable:4251)

//#define BOOST_ALL_DYN_LINK

#include "IceLeakWatch.h"
#include "OgreString.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"

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

#define ICE_SAFEDELETE(object) if (object) { Ice::LeakManager::getInstance()= object; object = nullptr; }

#define ICE_NOASSERRTS 0
#define ICE_NOWARNINGS 0
#define ICE_NONOTES 0

#define STRINGIFY(param) #param

#define ITERATE(iter, container) for (auto iter = container.begin(); iter != container.end(); iter++)

#ifdef _DEBUG
	#define IceAssert(expression) \
	if (!(expression)) \
	{ \
		Ogre::LogManager::getSingleton().logMessage(Ogre::String("Assertion failed: ") + #expression + Ogre::String(", function ") + __FUNCTION__ + Ogre::String(", file ") + __FILE__ +  Ogre::String(", line ") + Ogre::StringConverter::toString(__LINE__)); \
		__debugbreak();	\
	}
#else
	#define IceAssert(expression) \
	if (!(expression)) \
	{ \
		Ogre::LogManager::getSingleton().logMessage(Ogre::String("Assertion failed: ") + #expression + Ogre::String(", function ") + __FUNCTION__ + Ogre::String(", file ") + __FILE__ +  Ogre::String(", line ") + Ogre::StringConverter::toString(__LINE__)); \
	}
#endif

#if ICE_NOWARNINGS
	#define IceWarning(message)
#else
	#define IceWarning(message) Ogre::LogManager::getSingleton().logMessage(Ogre::String("Warning: ") + message + Ogre::String(" (") + __FUNCTION__ + Ogre::String(", file ") + __FILE__ +  Ogre::String(")"));
#endif

#if ICE_NONOTES
	#define IceNote(message)
#else
	#define IceNote(message) Ogre::LogManager::getSingleton().logMessage(message + Ogre::String(", function ") + __FUNCTION__);
#endif


#if ICE_NOASSERRTS
	#undef IceAssert
	#define IceAssert(expression)
#endif

namespace Ice
{
	enum VisibilityFlags
	{
		V_REFRACTIVE = 0x00000001,
		V_VOLUMETRIC = 0x00000002,
		V_EDITOROBJECT = 0x00000004,
		V_SKY = 0x00000008,
		V_GUI = 0x00000010,
		V_DEFAULT = 0xFFFFFFE0
	};

	enum AccessPermitions
	{
		ACCESS_VIEW = 0,		//access Ogre
		ACCESS_PHYSICS = 1,		//access physics engine
		ACCESS_NONE = 2,		//access nothing, only perform own computations (ai)
		ACCESS_ALL = 3			//access everything (scripting)
	};

	enum GlobalMessageIDs
	{
		UPDATE_PER_FRAME = 0, RENDERING_BEGIN = 1,										//called per frame
		PHYSICS_BEGIN = 2, PHYSICS_SUBSTEP = 3, PHYSICS_END = 4,						//called per physics update (can be multiple times per frame)
		KEY_DOWN = 20, KEY_UP = 21, MOUSE_DOWN = 22, MOUSE_UP = 23, MOUSE_MOVE = 24,	//user input
		ACTOR_ONSLEEP = 30, ACTOR_ONWAKE = 31, MATERIAL_ONCONTACT = 32,					//physics callbacks
		REPARSE_SCRIPTS_PRE = 40, REPARSE_SCRIPTS_POST = 41,							//reload scripts, reload materials
		LOADLEVEL_BEGIN = 50, LOADLEVEL_END = 51,	SAVELEVEL_BEGIN = 52, SAVELEVEL_END = 53,
		ENABLE_GAME_CLOCK = 60,
		GAMESTATE_ENTER = 10, GAMESTATE_LEAVE = 11,
		CONSOLE_INGAME = 70
	};

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
	class FreeFlightCameraController;
	class ObjectReference;
	class MainLoopThread;

	typedef DllExport std::shared_ptr<ObjectReference> ObjectReferencePtr;
	typedef DllExport std::shared_ptr<GameObject> GameObjectPtr;
	typedef DllExport std::shared_ptr<GOComponent> GOComponentPtr;
};