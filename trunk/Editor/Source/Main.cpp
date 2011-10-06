
#include "IceMain.h"
#include "wxEdit.h"
#include "IceSceneManager.h"
#include "Edit.h"
#include "IceMainLoop.h"

//Editor Modus
class Main : public wxApp
{
public:
	wxEdit* mFrame;
	bool OnInit()
	{
#if		_DEBUG
		Ogre::Root* ogre = new Ogre::Root("","","ogre.graphics.log");
#else
		Ogre::Root* ogre = new Ogre::Root("","","ogre.graphics.log");
#endif
		/*Ogre::RenderSystemList *renderSystems = NULL;
		Ogre::RenderSystemList::iterator r_it;
		renderSystems = ogre->getAvailableRenderers();
		r_it = renderSystems->begin();
		ogre->setRenderSystem(*r_it);*/

		Ice::Log::Instance().LogMessage("new wxEdit");
		Ice::Main::Instance().ExternInit();
		mFrame = new wxEdit();
		SetTopWindow(mFrame);
		mFrame->Show();
		Ice::Main::Instance().Run(mFrame->GetOgrePane()->getRenderWindow(), (size_t)((HWND)mFrame->GetHandle()));
		mFrame->PostCreate();

		//Ice::Main::Instance().CreateMainLoopThreads();
		Ice::MainLoopThread *physicsThread = new Ice::PhysicsThread();
		physicsThread->SetFixedTimeStep(20);
		Ice::Main::Instance().AddMainLoopThread("Physics", physicsThread);

		Ice::MainLoopThread *renderThread = new Ice::RenderThread();
		Ice::Main::Instance().AddMainLoopThread("View", renderThread, false);

		Ice::MainLoopThread *indyThread = new Ice::IndependantThread();//MainLoopThread(Ice::AccessPermissions::ACCESS_NONE);
		indyThread->SetFixedTimeStep(30);
		Ice::Main::Instance().AddMainLoopThread("Independant", indyThread);

		Ice::MainLoopThread *synchronized = new Ice::SynchronisedThread();
		synchronized->SetSynchronized(true);
		Ice::Main::Instance().AddMainLoopThread("Synchronized", synchronized, false);

		return true;                    
	}

	int OnExit()
	{
		std::cout << "OnExit" << std::endl;
		//Ice::Main::Instance().Shutdown();
		return 0;
	}
};
IMPLEMENT_APP_NO_MAIN(Main);

int main(int argc, char *argv[])
{
	return wxEntry(argc, argv);
};

int WINAPI WinMain(HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
					 LPSTR lpCmdLine,
					 int nCmdShow )
{
	return wxEntry(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
};