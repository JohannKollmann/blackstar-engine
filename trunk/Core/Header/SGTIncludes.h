

#ifndef __SGTIncludes_H__
#define __SGTIncludes_H__

#define SGTDllExport __declspec(dllexport)

#pragma warning(disable:4482)

#define BOOST_ALL_DYN_LINK

#define SCRIPT_BASE_DIR Ogre::String("Data/Scripts/Game/")

class SGTMain;
class SGTKernel;
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
class SGTAIManager;

#endif