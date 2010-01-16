
#pragma once

#include "IceIncludes.h"
#include "Ogre.h"
#include "IceMessageListener.h"
#include "IceScriptSystem.h"


namespace Ice
{

class DllExport Console : public MessageListener
{
private:
	std::map<Ogre::String, std::vector<Ogre::String> > mCommands;
	static std::vector<ScriptParam> Lua_GetNumCommands(Script& caller, std::vector<ScriptParam> vParams);
	static std::vector<ScriptParam> Lua_GetCommand(Script& caller, std::vector<ScriptParam> vParams);
	static std::vector<ScriptParam> Lua_ExecCommand(Script& caller, std::vector<ScriptParam> vParams);

public:
	Console();
	~Console();

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

	void ReceiveMessage(Msg &msg);

	//Singleton
	static Console& Instance();
};

};