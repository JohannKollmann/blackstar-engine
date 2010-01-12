
#include "..\Header\SGTConsole.h"
#include "SGTMessageSystem.h"
#include "SGTMain.h"
#include "SGTInput.h"
#include "SGTScriptSystem.h"
#include "SGTSceneManager.h"
#include "SGTUtils.h"

#define CONSOLE_LINE_LENGTH 85
#define CONSOLE_LINE_COUNT 8 

SGTConsole::SGTConsole()
{
	SGTMessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
	SGTMessageSystem::Instance().JoinNewsgroup(this, "CONSOLE_INGAME");
	AddCommand("lua_loadscript", "string");

	SGTScriptSystem::GetInstance().ShareCFunction("console_get_num_commands", Lua_GetNumCommands);
	SGTScriptSystem::GetInstance().ShareCFunction("console_get_command", Lua_GetCommand);
	SGTScriptSystem::GetInstance().ShareCFunction("console_exec_command", Lua_ExecCommand);
}

SGTConsole::~SGTConsole()
{
}

void SGTConsole::Init()
{
}

void SGTConsole::Shutdown()
{
}

void SGTConsole::Show(bool show)
{
}

void SGTConsole::ReceiveMessage(SGTMsg &msg)
{
	if (msg.mNewsgroup == "CONSOLE_INGAME")
	{
		if (msg.mData.GetOgreString("COMMAND") == "lua_loadscript")
		{
			int counter = 0;
			Ogre::String scriptfile = "";
			Ogre::String funcname = "";
			std::vector<SGTScriptParam> params;
			while (msg.mData.HasNext())
			{
				SGTGenericProperty entry = msg.mData.GetNext();
				if (counter == 0)
				{
					counter = 1;
					continue;
				}
				if (counter == 1)
				{
					if (entry.mType == "Ogre::String")
					{
						scriptfile = Ogre::any_cast<Ogre::String>(entry.mData);
						counter = 2;
						continue;
					}
				}
				if (counter == 2)
				{
					if (entry.mType == "Ogre::String")
					{
						funcname = Ogre::any_cast<Ogre::String>(entry.mData);
						counter = 3;
						continue;
					}
				}
				if (counter == 3)
				{
					if (entry.mType == "bool") params.push_back(SGTScriptParam(Ogre::any_cast<bool>(entry.mData)));
					if (entry.mType == "float") params.push_back(SGTScriptParam(Ogre::any_cast<float>(entry.mData)));
					if (entry.mType == "Ogre::String")
					{
						std::string str = Ogre::any_cast<Ogre::String>(entry.mData);
						params.push_back(SGTScriptParam(str));
					}
				}
			}
			if (scriptfile != "" && funcname != "")
			{
				SGTScript script = SGTScriptSystem::GetInstance().CreateInstance(scriptfile);
				script.CallFunction(funcname, params);
			}
			
		}
		
	}
}

void SGTConsole::Print(Ogre::String text)
{
}

void SGTConsole::ExecCommand(Ogre::String command)
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
			SGTMsg msg;
			msg.mNewsgroup = "CONSOLE_INGAME";
			msg.mData.AddOgreString("COMMAND", (*i).first);
			for (unsigned int paramindex = 1; paramindex < inputs.size(); paramindex++)
			{
				Ogre::String paramname = "PARAM" + Ogre::StringConverter::toString(paramindex);
				msg.mData.AddOgreString(paramname, inputs[paramindex]);
			}
			SGTMessageSystem::Instance().SendMessage(msg);
			Print(command);
			return;
		}
	}
	//check if it was a lua function
	std::vector<std::string> vstrFunctions=SGTScriptSystem::GetInstance().GetFunctionNames();
	for(unsigned int iCommand=0; iCommand<vstrFunctions.size(); iCommand++)
	{
		if(inputs[0]==vstrFunctions[iCommand])
		{
			std::vector<SGTScriptParam> vParams;
			for(unsigned int i=1; i<inputs.size(); i++)
				vParams.push_back(SGTScriptParam(inputs[i]));
			SGTScriptSystem::GetInstance().RunFunction(inputs[0], vParams);
			return;
		}
	}
	Print("Error: Unknown Command!");
	Ogre::LogManager::getSingleton().logMessage("SGTConsole::ExecCommand: Unknown Command (" + command + ")");

}

void SGTConsole::AddCommand(Ogre::String name, Ogre::String parameters)
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

unsigned int SGTConsole::GetNumCommands()
{
	return mCommands.size() + SGTScriptSystem::GetInstance().GetFunctionNames().size();
}
Ogre::String SGTConsole::GetCommand(unsigned int index)
{
	
	unsigned int i=0; 
	for (std::map<Ogre::String,std::vector<Ogre::String>>::iterator it=mCommands.begin(); it!=mCommands.end(); i++, it++)
		if(i==index)
			return it->first;

	std::vector<std::string> vstrFunctions=SGTScriptSystem::GetInstance().GetFunctionNames();
	for(unsigned int iFunction=0; iFunction<vstrFunctions.size(); iFunction++, i++)
		if(i==index)
			return vstrFunctions[iFunction];

	return Ogre::String();
}

SGTConsole& SGTConsole::Instance()
{
	static SGTConsole TheOneAndOnly;
	return TheOneAndOnly;
};

std::vector<SGTScriptParam>
SGTConsole::Lua_GetNumCommands(SGTScript &caller, std::vector<SGTScriptParam> vParams)
{
	return std::vector<SGTScriptParam>(1, SGTScriptParam((int)Instance().GetNumCommands()));
}

std::vector<SGTScriptParam>
SGTConsole::Lua_GetCommand(SGTScript &caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> errout(1, SGTScriptParam());
	std::vector<SGTScriptParam> vRef=std::vector<SGTScriptParam>(1, SGTScriptParam(0.1));

	std::string strErrString=SGTUtils::TestParameters(vParams, vRef, false);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	
	return std::vector<SGTScriptParam>(1, SGTScriptParam(Instance().GetCommand((int)vParams[0].getFloat())));
}

std::vector<SGTScriptParam>
SGTConsole::Lua_ExecCommand(SGTScript &caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> errout(1, SGTScriptParam());
	std::vector<SGTScriptParam> vRef=std::vector<SGTScriptParam>(1, SGTScriptParam(std::string()));

	std::string strErrString=SGTUtils::TestParameters(vParams, vRef, false);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	Instance().ExecCommand(vParams[0].getString());
	
	return std::vector<SGTScriptParam>();
}

/*
std::vector<SGTScriptParam>
SGTGUISystem::Lua_MoveWindow(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> errout;
	errout.push_back(SGTScriptParam());
	std::vector<SGTScriptParam> vRef=std::vector<SGTScriptParam>(1, SGTScriptParam(0.1));
	vRef.push_back(SGTScriptParam(0.1));
	vRef.push_back(SGTScriptParam(0.1));

	std::string strErrString=SGTUtils::TestParameters(vParams, vRef, false);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	
	std::map<int, SWindowInfo>::const_iterator it=GetInstance().m_mWindowInfos.find((int)vParams[0].getFloat());
	if(it==GetInstance().m_mWindowInfos.end())
	{
		errout.push_back(SGTScriptParam(std::string("could not find a window with given ID")));
		return errout;
	}
	SGTGUISystem::SWindowInfo wininfo=it->second;
	int iHandle=it->first;
	while(wininfo.iParentHandle!=-1)
	{
		wininfo=SGTGUISystem::GetInstance().m_mWindowInfos.find(wininfo.iParentHandle)->second;
		iHandle=wininfo.iParentHandle;
	}
	Window(iHandle).Move(vParams[1].getFloat(), vParams[2].getFloat());
	return std::vector<SGTScriptParam>();
}*/