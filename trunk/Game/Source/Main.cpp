

#include "SGTMain.h"
#include "SGTMainLoop.h"
#define BOOST_ALL_DYN_LINK

//Yes, it is really basic

int main (int argc, char *argv[])
{
	SGTMain::Instance().Run();
	SGTMainLoop::Instance().startLoop();
	SGTMain::Instance().Shutdown();
};

int WINAPI WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine,
                     int nCmdShow )
{
	SGTMain::Instance().Run();
	SGTMainLoop::Instance().startLoop();
	SGTMain::Instance().Shutdown();
}