
#include "..\Header\SGTConsole.h"
#include "SGTMessageSystem.h"
#include "SGTMain.h"
#include "SGTInput.h"


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
	if (mActive)
	{
		if (msg.mNewsgroup == "KEY_DOWN")
		{
			unsigned int kc = *(unsigned int*)(msg.mData.GetInt("KEY_ID"));
			OIS::KeyCode okc = *(OIS::KeyCode*)(msg.mData.GetInt("KEY_ID_OIS"));

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
			if ((*i).second.size() > inputs.size() - 1)
			{
				Print("Error: Not enough parameters!");
				Ogre::LogManager::getSingleton().logMessage("SGTConsole::ExecCommand: Not enough parameters in command: " + command);
			}
			else if ((*i).second.size() < inputs.size() - 1)
			{
				Print("Error: Too many parameters!");
				Ogre::LogManager::getSingleton().logMessage("SGTConsole::ExecCommand: Too many parameters in command: " + command);
			}
			else
			{
				int paramindex = 1;
				SGTMsg msg;
				msg.mNewsgroup = "CONSOLE_INGAME";
				msg.mData.AddOgreString("COMMAND", (*i).first);
				std::vector<Ogre::String>::iterator param = inputs.begin();
				param++;
				for (std::vector<Ogre::String>::iterator typeiter = (*i).second.begin(); typeiter != (*i).second.end(); typeiter++)
				{
					Ogre::String paramname = "PARAM" + Ogre::StringConverter::toString(paramindex);
					if ((*typeiter) == "int") msg.mData.AddInt(paramname, Ogre::StringConverter::parseInt((*param)));
					if ((*typeiter) == "float") msg.mData.AddFloat(paramname, Ogre::StringConverter::parseReal((*param)));
					if ((*typeiter) == "bool") msg.mData.AddBool(paramname, Ogre::StringConverter::parseBool((*param)));
					if ((*typeiter) == "vector3")
					{
						std::replace((*param).begin(), (*param).end(), '/', ' ');
						msg.mData.AddOgreVec3(paramname, Ogre::StringConverter::parseVector3((*param)));
					}
					param++;
					paramindex++;
				}
				SGTMessageSystem::Instance().SendMessage(msg);
				Print(command);
			}
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