

#include "IceMain.h"
#define BOOST_ALL_DYN_LINK

//Yes, it is really basic

int main (int argc, char *argv[])
{
	Ice::Main::Instance().Run();
	Ice::Main::Instance().Shutdown();
};

int WINAPI WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine,
                     int nCmdShow )
{
	Ice::Main::Instance().Run();
	Ice::Main::Instance().Shutdown();
}