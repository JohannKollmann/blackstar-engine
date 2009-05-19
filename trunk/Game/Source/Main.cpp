

#include "SGTMain.h"
#include "SGTGameState.h"
#define BOOST_ALL_DYN_LINK

//Yes, it is really basic

int main (int argc, char *argv[])
{
	SGTMain::Instance().Run();
	SGTKernel::Instance().startLoop();
	SGTMain::Instance().Shutdown();
};