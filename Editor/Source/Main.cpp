
#include "IceMain.h"
#include "IceGamestate.h"
#include "wxEdit.h"
#include "IceSceneManager.h"
#include "Edit.h"

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

		Ogre::LogManager::getSingleton().logMessage("new wxEdit");
		Ice::Main::Instance().ExternInit();
		mFrame = new wxEdit();
		SetTopWindow(mFrame);
		mFrame->Show();
		Ice::MessageSystem::CreateNewsgroup(GlobalMessageIDs::REPARSE_SCRIPTS_PRE);
		Ice::MessageSystem::CreateNewsgroup(GlobalMessageIDs::REPARSE_SCRIPTS_POST);
		Ice::Main::Instance().Run(mFrame->GetOgrePane()->getRenderWindow(), (size_t)((HWND)mFrame->GetHandle()));
		mFrame->PostCreate();
		//Ogre::LogManager::getSingleton().logMessage("Root Num Children: " + Ogre::StringConverter::toString(Ice::Main::Instance().GetOgreSceneMgr()->getRootSceneNode()->numChildren()));
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