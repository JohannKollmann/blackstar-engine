
#include "IceConsole.h"
#include "IceMessageSystem.h"
#include "IceMain.h"
#include "IceInput.h"
#include "IceScriptSystem.h"
#include "IceSceneManager.h"
#include "IceUtils.h"

#define CONSOLE_LINE_LENGTH 85
#define CONSOLE_LINE_COUNT 8 

namespace Ice
{

Console::Console()
{
	JoinNewsgroup(GlobalMessageIDs::UPDATE_PER_FRAME);
	JoinNewsgroup(GlobalMessageIDs::CONSOLE_INGAME);
	AddCommand("lua_loadscript", "string");

	ScriptSystem::GetInstance().ShareCFunction("console_get_num_commands", Lua_GetNumCommands);
	ScriptSystem::GetInstance().ShareCFunction("console_get_command", Lua_GetCommand);
	ScriptSystem::GetInstance().ShareCFunction("console_exec_command", Lua_ExecCommand);
}

Console::~Console()
{
}

void Console::Init()
{
}

void Console::Shutdown()
{
}

void Console::Show(bool show)
{
}

void Console::ReceiveMessage(Msg &msg)
{
	//Todo
	if (msg.typeID == GlobalMessageIDs::CONSOLE_INGAME)
	{
		/*if (msg.params.GetOgreString("COMMAND") == "lua_loadscript")
		{
			int counter = 0;
			Ogre::String scriptfile = "";
			Ogre::String funcname = "";
			std::vector<ScriptParam> params;
			while (msg.params.HasNext())
			{
				GenericProperty entry = msg.params.GetNext();
				if (counter == 0)
				{
					counter = 1;
					continue;
				}
				if (counter == 1)
				{
					if (entry.mType == "Ogre::String")
					{
						scriptfile = entry.Get<Ogre::String>();
						counter = 2;
						continue;
					}
				}
				if (counter == 2)
				{
					if (entry.mType == "Ogre::String")
					{
						funcname = entry.Get<Ogre::String>();
						counter = 3;
						continue;
					}
				}
				if (counter == 3)
				{
					if (entry.mType == "bool") params.push_back(ScriptParam(entry.Get<bool>()));
					if (entry.mType == "float") params.push_back(ScriptParam(entry.Get<float>()));
					if (entry.mType == "Ogre::String")
					{
						std::string str = entry.Get<Ogre::String>();
						params.push_back(ScriptParam(str));
					}
				}
			}
			if (scriptfile != "" && funcname != "")
			{
				Script script = ScriptSystem::GetInstance().CreateInstance(scriptfile);
				script.CallFunction(funcname, params);
			}
			
		}*/
		
	}
}

void Console::Print(Ogre::String text)
{
}

void Console::ExecCommand(Ogre::String command)
{
	if(!command.length())
		return;
	std::vector<Ogre::String> inputs;
	Ogre::String current = "";
	bool bQuoted=false;
	for (unsigned int i = 0; i < command.length(); i++)
	{
		if(command[i]=='\"' && bQuoted)
		{
			bQuoted=false;
			continue;
		}
		if(command[i]=='\"' && current.length()==0)
		{
			bQuoted=true;
			continue;
		}
		
		if (command[i] == ' ' && !bQuoted)
		{
			if (current.length() > 0)
			{
				inputs.push_back(current);
				current = "";
			}
		}
		else current += command[i];
	}
	if (current != "") inputs.push_back(current);
 
	for (std::map<Ogre::String, std::vector<Ogre::String> >::iterator i = mCommands.begin(); i != mCommands.end(); i++)
	{
		if ((*i).first == inputs[0])
		{
			Msg msg;
			msg.typeID = GlobalMessageIDs::CONSOLE_INGAME;
			msg.params.AddOgreString("COMMAND", (*i).first);
			for (unsigned int paramindex = 1; paramindex < inputs.size(); paramindex++)
			{
				Ogre::String paramname = "PARAM" + Ogre::StringConverter::toString(paramindex);
				msg.params.AddOgreString(paramname, inputs[paramindex]);
			}
			MulticastMessage(msg);
			Print(command);
			return;
		}
	}
	//check if it was a lua function
	std::vector<std::string> vstrFunctions=ScriptSystem::GetInstance().GetFunctionNames();
	for(unsigned int iCommand=0; iCommand<vstrFunctions.size(); iCommand++)
	{
		if(inputs[0]==vstrFunctions[iCommand])
		{
			std::vector<ScriptParam> vParams;
			for(unsigned int i=1; i<inputs.size(); i++)
				vParams.push_back(ScriptParam(inputs[i]));
			ScriptSystem::GetInstance().RunFunction(inputs[0], vParams);
			return;
		}
	}
	Print("Error: Unknown Command!");
	Ogre::LogManager::getSingleton().logMessage("Console::ExecCommand: Unknown Command (" + command + ")");

}

void Console::AddCommand(Ogre::String name, Ogre::String parameters)
{
	std::vector<Ogre::String> types;
	Ogre::String current = "";
	for (unsigned int i = 0; i < parameters.length(); i++)
	{
		if (parameters[i] == ' ')
		{
			if (current.length() > 0)
			{
				types.push_back(current);
				current = "";
			}
		}
		else current += parameters[i];
	}
	if (current != "") types.push_back(current);

	mCommands.insert(std::make_pair(name, types));
}

unsigned int Console::GetNumCommands()
{
	return mCommands.size() + ScriptSystem::GetInstance().GetFunctionNames().size();
}
Ogre::String Console::GetCommand(unsigned int index)
{
	
	unsigned int i=0; 
	for (std::map<Ogre::String,std::vector<Ogre::String>>::iterator it=mCommands.begin(); it!=mCommands.end(); i++, it++)
		if(i==index)
			return it->first;

	std::vector<std::string> vstrFunctions=ScriptSystem::GetInstance().GetFunctionNames();
	for(unsigned int iFunction=0; iFunction<vstrFunctions.size(); iFunction++, i++)
		if(i==index)
			return vstrFunctions[iFunction];

	return Ogre::String();
}

Console& Console::Instance()
{
	static Console TheOneAndOnly;
	return TheOneAndOnly;
};

std::vector<ScriptParam>
Console::Lua_GetNumCommands(Script &caller, std::vector<ScriptParam> vParams)
{
	return std::vector<ScriptParam>(1, ScriptParam((int)Instance().GetNumCommands()));
}

std::vector<ScriptParam>
Console::Lua_GetCommand(Script &caller, std::vector<ScriptParam> vParams)
{
	std::vector<ScriptParam> errout(1, ScriptParam());
	std::vector<ScriptParam> vRef=std::vector<ScriptParam>(1, ScriptParam(0.1));

	std::string strErrString=Utils::TestParameters(vParams, vRef, false);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	
	return std::vector<ScriptParam>(1, ScriptParam(Instance().GetCommand((int)vParams[0].getFloat())));
}

std::vector<ScriptParam>
Console::Lua_ExecCommand(Script &caller, std::vector<ScriptParam> vParams)
{
	std::vector<ScriptParam> errout(1, ScriptParam());
	std::vector<ScriptParam> vRef=std::vector<ScriptParam>(1, ScriptParam(std::string()));

	std::string strErrString=Utils::TestParameters(vParams, vRef, false);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	Instance().ExecCommand(vParams[0].getString());
	
	return std::vector<ScriptParam>();
}

/*
std::vector<ScriptParam>
GUISystem::Lua_MoveWindow(Script& caller, std::vector<ScriptParam> vParams)
{
	std::vector<ScriptParam> errout;
	errout.push_back(ScriptParam());
	std::vector<ScriptParam> vRef=std::vector<ScriptParam>(1, ScriptParam(0.1));
	vRef.push_back(ScriptParam(0.1));
	vRef.push_back(ScriptParam(0.1));

	std::string strErrString=Utils::TestParameters(vParams, vRef, false);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	
	std::map<int, SWindowInfo>::const_iterator it=GetInstance().m_mWindowInfos.find((int)vParams[0].getFloat());
	if(it==GetInstance().m_mWindowInfos.end())
	{
		errout.push_back(ScriptParam(std::string("could not find a window with given ID")));
		return errout;
	}
	GUISystem::SWindowInfo wininfo=it->second;
	int iHandle=it->first;
	while(wininfo.iParentHandle!=-1)
	{
		wininfo=GUISystem::GetInstance().m_mWindowInfos.find(wininfo.iParentHandle)->second;
		iHandle=wininfo.iParentHandle;
	}
	Window(iHandle).Move(vParams[1].getFloat(), vParams[2].getFloat());
	return std::vector<ScriptParam>();
}*/

};