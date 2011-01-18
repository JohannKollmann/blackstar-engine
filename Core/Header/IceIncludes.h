

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
		V_DEFAULT = 0xFFFFFF0,
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
};