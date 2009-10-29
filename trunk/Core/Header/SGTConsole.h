
#ifndef __SGTConsole_H__
#define __SGTConsole_H__

#include "SGTIncludes.h"
#include "Ogre.h"
#include "SGTMessageListener.h"
#include "SGTScriptSystem.h"


class SGTDllExport SGTConsole : public SGTMessageListener
{
private:
	std::map<Ogre::String, std::vector<Ogre::String> > mCommands;
	static std::vector<SGTScriptParam> Lua_GetNumCommands(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_GetCommand(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_ExecCommand(SGTScript& caller, std::vector<SGTScriptParam> vParams);

public:
	SGTConsole();
	~SGTConsole();

	void Init();
	void Shutdown();

	void Show(bool show);

	unsigned int GetNumCommands();
	Ogre::String GetCommand(unsigned int index);

	/*
	Benutzung:
	AddCommand("change_res", "int int");
	*/
	void AddCommand(Ogre::String name, Ogre::String parameters);

	void ExecCommand(Ogre::String command);

	void Print(Ogre::String text);

	void ReceiveMessage(SGTMsg &msg);

	//Singleton
	static SGTConsole& Instance();
};

#endif