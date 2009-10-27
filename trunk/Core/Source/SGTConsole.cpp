
#include "..\Header\SGTConsole.h"
#include "SGTMessageSystem.h"
#include "SGTMain.h"
#include "SGTInput.h"
#include "SGTScriptSystem.h"
#include "SGTSceneManager.h"

#define CONSOLE_LINE_LENGTH 85
#define CONSOLE_LINE_COUNT 8 

SGTConsole::SGTConsole()
{
	SGTMessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
	SGTMessageSystem::Instance().JoinNewsgroup(this, "CONSOLE_INGAME");
	AddCommand("lua_loadscript", "string");
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
	std::vector<Ogre::String> inputs;
	Ogre::String current = "";
	for (unsigned int i = 0; i < command.length(); i++)
	{
		if (command[i] == ' ')
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
	return mCommands.size();
}
Ogre::String SGTConsole::GetCommand(unsigned int index)
{
	std::map<Ogre::String, std::vector<Ogre::String> >::iterator i = mCommands.begin();
	for (unsigned int n = 0; n < index; n++) i++;
	return i->first;
}

SGTConsole& SGTConsole::Instance()
{
	static SGTConsole TheOneAndOnly;
	return TheOneAndOnly;
};