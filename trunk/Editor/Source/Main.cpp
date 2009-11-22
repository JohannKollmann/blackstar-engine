
#include "SGTMain.h"
#include "SGTGamestate.h"
#include "wxEdit.h"
#include "SGTSceneManager.h"

//Editor Modus
class Edit : public wxApp
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
		SGTMain::Instance().ExternInit();
		mFrame = &wxEdit::Instance();
		SetTopWindow(mFrame);
		mFrame->Show();
		CreateNewsgroup("REPARSE_SCRIPTS");
		SGTMain::Instance().Run(mFrame->GetOgrePane()->getRenderWindow(), (size_t)((HWND)mFrame->GetHandle()));
		mFrame->PostCreate();
		//Ogre::LogManager::getSingleton().logMessage("Root Num Children: " + Ogre::StringConverter::toString(SGTMain::Instance().GetOgreSceneMgr()->getRootSceneNode()->numChildren()));
		return true;                    
	}

	int OnExit()
	{
		std::cout << "OnExit" << std::endl;
		//SGTMain::Instance().Shutdown();
		return 0;
	}
};
IMPLEMENT_APP_NO_MAIN(Edit);

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