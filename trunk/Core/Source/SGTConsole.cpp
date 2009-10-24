
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
	mActive = false;
	mInitialized = false;
}

SGTConsole::~SGTConsole()
{
}

void SGTConsole::Init()
{
	if (!mInitialized)
	{
		mHeight=0.4;

		// Create background rectangle covering the whole screen
		mRect = new Ogre::Rectangle2D(true);
		mRect->setCorners(-1, 1, 1, 1-mHeight);
		mRect->setMaterial("console/background");
		mRect->setRenderQueueGroup(Ogre::RENDER_QUEUE_OVERLAY);
		mRect->setBoundingBox(Ogre::AxisAlignedBox(-100000.0*Ogre::Vector3::UNIT_SCALE, 100000.0*Ogre::Vector3::UNIT_SCALE));
		mNode = SGTMain::Instance().GetOgreSceneMgr()->getRootSceneNode()->createChildSceneNode("#Console");
		mNode->attachObject(mRect);
		mRect->setCastShadows(false);
	
		mTextbox=Ogre::OverlayManager::getSingleton().createOverlayElement("TextArea","ConsoleText");
		mTextbox->setCaption(">");
		mTextbox->setMetricsMode(Ogre::GMM_RELATIVE);
		mTextbox->setPosition(0,0);
		mTextbox->setParameter("font_name","Bluehigh");
		mTextbox->setParameter("colour_top","1 1 1");
		mTextbox->setParameter("colour_bottom","1 1 1");
		mTextbox->setParameter("char_height","0.025");
   
		mOverlay=Ogre::OverlayManager::getSingleton().create("Console");   
		mOverlay->add2D((Ogre::OverlayContainer*)mTextbox);
		mOverlay->show(); 

		SGTMessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
		SGTMessageSystem::Instance().JoinNewsgroup(this, "KEY_DOWN");
		SGTMessageSystem::Instance().JoinNewsgroup(this, "KEY_UP");
		SGTMessageSystem::Instance().JoinNewsgroup(this, "CONSOLE_INGAME");
		SGTConsole::Instance().AddCommand("lua_loadscript", "string");
		mInitialized = true;
	}
}

void SGTConsole::Shutdown()
{
   if(!mInitialized) return;
   SGTMain::Instance().GetOgreSceneMgr()->destroySceneNode(mNode);
   /*delete mRect;
   delete mNode;
   delete mTextbox;
   delete mOverlay;*/
}

void SGTConsole::Show(bool show)
{
	if (mInitialized)
	{
		mActive = show;
		mNode->setVisible(show);
		if (show) mOverlay->show();
		else mOverlay->hide();
	}
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
	if (msg.mNewsgroup == "KEY_UP")
	{
		OIS::KeyCode okc = (OIS::KeyCode)(msg.mData.GetInt("KEY_ID_OIS"));
		if (okc == OIS::KeyCode::KC_F1)
		{
			if (!mActive) SGTConsole::Instance().Show(true);
			else SGTConsole::Instance().Show(false);
		}
	}
	if (mActive)
	{
		if (msg.mNewsgroup == "KEY_DOWN")
		{
			unsigned int kc = msg.mData.GetInt("KEY_ID");
			OIS::KeyCode okc = (OIS::KeyCode)(msg.mData.GetInt("KEY_ID_OIS"));

			if (okc == OIS::KC_RETURN)
			{
				ExecCommand(mCurrentPrompt);
				mCurrentPrompt = "";
			}
			else if (okc == OIS::KC_BACK)
			{
				mCurrentPrompt = mCurrentPrompt.substr(0, mCurrentPrompt.length()-1);
			}
			else if (kc != 0)
			{
				mCurrentPrompt += kc;
			}
			mUpdateOverlay = true;
		}

		if (msg.mNewsgroup == "UPDATE_PER_FRAME")
		{
			if (mUpdateOverlay)
			{
				Ogre::String text;
				std::list<Ogre::String>::iterator i,start,end; 
				//make sure is in range
				if(mStartLine>mLines.size())
					mStartLine=mLines.size();

				int lcount=0;
				start=mLines.begin();
				for(unsigned int c=0;c<mStartLine;c++)
					start++;
				end=start;
				for(unsigned int c=0;c<CONSOLE_LINE_COUNT;c++){
					if(end==mLines.end())
						break;
					end++;
				}
				for(i=start;i!=end;i++)
					text+=(*i)+"\n";
      
				//add the prompt
				text+=("> "+mCurrentPrompt);

				mTextbox->setCaption(text); 

				mUpdateOverlay = false;
			}
		}
	}
}

void SGTConsole::Print(Ogre::String text)
{
	if (mActive)
	{
   //subdivide it into lines
   const char *str=text.c_str();
   int start=0,count=0;
   unsigned int len=text.length();
   Ogre::String line;
   for(unsigned int c=0;c<len;c++)
   {
      if(str[c]=='\n'||line.length()>=CONSOLE_LINE_LENGTH)
	  {
         mLines.push_back(line);
         line="";
      }
      if(str[c]!='\n')
         line+=str[c];
   }
   if(line.length())
      mLines.push_back(line);
   if(mLines.size()>CONSOLE_LINE_COUNT)
	   mStartLine=mLines.size()-CONSOLE_LINE_COUNT;
   else
      mStartLine=0;

   mUpdateOverlay = true;
	}
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

SGTConsole& SGTConsole::Instance()
{
	static SGTConsole TheOneAndOnly;
	return TheOneAndOnly;
};