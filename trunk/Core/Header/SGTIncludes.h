

#ifndef __SGTIncludes_H__
#define __SGTIncludes_H__

#define SGTDllExport __declspec(dllexport)

#pragma warning(disable:4482)
#pragma warning(disable:4251)

//#define BOOST_ALL_DYN_LINK

namespace NxOgre
{
	class World;
	class Scene;
};
namespace OgreOggSound
{
	class OgreOggSoundManager;
};
class NxControllerManager;

class SGTMain;
class SGTMainLoop;
class SGTSceneManager;
class SGTLevelMesh;
class SGTInput;
class SGTCameraController;
class SGTMsg;
class SGTDataMap;
class SGTMessageSystem;
class SGTMessageListener;
class SGTObjectLevelRayCaster;
class SGTCameraController;
class SGTWeatherController;
class SGTCompositorLoader;
class SGTCollisionCallback;
class SGTGameObject;
class SGTGOComponent;
class ComponentSection;
struct SGTObjectMsg;
class SGTGOCViewListener;
class SGTGOCAI;
class SGTScriptedAIState;
class SGTDayCycle;
class SGTAIManager;

#endif