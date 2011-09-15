

#pragma once

#pragma warning(disable:4482)		//"nonstandard expression used in qualified name"
#pragma warning(disable:4305)		//implicit cast from double to float
#pragma warning(disable:4244)		//cast from double to const float
#pragma warning(disable:4251)

//#define BOOST_ALL_DYN_LINK

#include "IceLeakWatch.h"
#include "OgreString.h"
#include "OgreStringConverter.h"
#include "IceLog.h"
#include "IceTextIDs.h"


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

#define ITERATE(iter, container) for (auto iter = container.begin(); iter != container.end(); ++iter)

#ifdef _DEBUG
	#define IceAssert(expression) \
	if (!(expression)) \
	{ \
		Ice::Log::Instance().LogCriticalError(Ogre::String("Assertion failed: ") + #expression + Ogre::String(", function ") + __FUNCTION__ + Ogre::String(", file ") + __FILE__ +  Ogre::String(", line ") + Ogre::StringConverter::toString(__LINE__)); \
		__debugbreak();	\
	}
#else
	#define IceAssert(expression) \
	if (!(expression)) \
	{ \
		Ice::Log::Instance().LogCriticalError(Ogre::String("Assertion failed: ") + #expression + Ogre::String(", function ") + __FUNCTION__ + Ogre::String(", file ") + __FILE__ +  Ogre::String(", line ") + Ogre::StringConverter::toString(__LINE__)); \
	}
#endif

#if ICE_NOWARNINGS
	#define IceWarning(message)
#else
	#define IceWarning(message) Ice::Log::Instance().LogWarning(message + Ogre::String(" (") + __FUNCTION__ + Ogre::String(", file ") + __FILE__ +  Ogre::String(")"));
#endif

#if ICE_NONOTES
	#define IceNote(message)
#else
	#define IceNote(message) Ice::Log::Instance().LogMessage(message + Ogre::String(", function ") + __FUNCTION__);
#endif


#if ICE_NOASSERRTS
	#undef IceAssert
	#define IceAssert(expression)
#endif

#define GLOBAL_MESSAGE_ID_FIRST 0x1000

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

	enum AccessPermissions
	{
		ACCESS_VIEW = 0,		//access Ogre
		ACCESS_PHYSICS = 1,		//access physics engine
		ACCESS_NONE = 2,		//access nothing, only perform own computations (ai)
		ACCESS_ALL = 3			//access everything (scripting)
	};



	enum GlobalMessageIDs
	{
		UPDATE_PER_FRAME = GLOBAL_MESSAGE_ID_FIRST, RENDERING_BEGIN,										//called per frame
		PHYSICS_BEGIN, PHYSICS_SUBSTEP, PHYSICS_END,						//called per physics update (can be multiple times per frame)
		KEY_DOWN, KEY_UP, MOUSE_DOWN, MOUSE_UP, MOUSE_MOVE,	//user input
		ACTOR_ONSLEEP, ACTOR_ONWAKE, MATERIAL_ONCONTACT,					//physics callbacks
		REPARSE_SCRIPTS_PRE, REPARSE_SCRIPTS_POST ,							//reload scripts, reload materials
		LOADLEVEL_BEGIN, LOADLEVEL_END,	SAVELEVEL_BEGIN, SAVELEVEL_END,
		ENABLE_GAME_CLOCK,
		GAMESTATE_ENTER, GAMESTATE_LEAVE,
		CONSOLE_INGAME,
		GLOBAL_MESSAGE_ID_LAST
	};
	//class TextID;
	TextID& getGlobalMessageIDs();

	#define DllExport __declspec(dllexport)

	class Main;
	class MainLoop;
	class SceneManager;
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
	class AIManager;
	class AStarEdge;
	class AStarNode;
	class MaterialTable;
	class GOCSimpleCameraController;
	class GOCCameraController;
	class FreeFlightCameraController;
	class ObjectReference;
	class MainLoopThread;

	typedef DllExport std::shared_ptr<ObjectReference> ObjectReferencePtr;
	typedef DllExport std::shared_ptr<GameObject> GameObjectPtr;
	typedef DllExport std::shared_ptr<GOComponent> GOComponentPtr;
};