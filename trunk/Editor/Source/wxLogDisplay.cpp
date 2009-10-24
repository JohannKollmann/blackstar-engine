
#include "wxLogDisplay.h"
#include "wxEdit.h"
#include "SGTSceneManager.h"

IMPLEMENT_CLASS(wxLogDisplay, wxListCtrl)

BEGIN_EVENT_TABLE(wxLogDisplay, wxListCtrl)
	EVT_LIST_ITEM_ACTIVATED(wxID_ANY, wxLogDisplay::OnActivated)
	EVT_SHOW(wxLogDisplay::OnShow)
END_EVENT_TABLE()

wxLogDisplay::wxLogDisplay(wxWindow *parent)
	: wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_NO_HEADER)
{
	Ogre::LogManager::getSingleton().getDefaultLog()->addListener(this);
	mIndex = 0;
    wxListItem itemCol;
	itemCol.SetWidth(800);
    itemCol.SetImage(-1);
    InsertColumn(0, itemCol);
}

wxLogDisplay::~wxLogDisplay(void)
{
}

void wxLogDisplay::OnActivated(wxListEvent& event)
{
	wxString msg = GetItemText(event.GetIndex());
	if (msg.find("[Script]") != wxString::npos)
	{
		size_t first = msg.find("\"");
		if (first != wxString::npos)
		{
			wxString scriptFile = msg.SubString(first, msg.find("\"", first+1));
			wxMainNotebook *notebook = wxEdit::Instance().GetMainNotebook();
			Ogre::String scriptPath = SGTSceneManager::Instance().GetScriptPath(scriptFile.c_str());
			notebook->AddScriptTab(scriptFile.c_str(), scriptPath);

			size_t line_index = msg.find(", line ");
			if (line_index != wxString::npos)
			{
				wxString sLine = msg.SubString(msg.find(", line ") + 7, msg.Len());
				Ogre::String oLine = sLine.c_str();
				int line = Ogre::StringConverter::parseInt(oLine);
				//jump to line
			}
		}
	}
}

void wxLogDisplay::messageLogged(const Ogre::String &message, Ogre::LogMessageLevel level, bool maskDebug, const Ogre::String &logName)
{
	if (level == Ogre::LogMessageLevel::LML_CRITICAL || message.find("error") != Ogre::String::npos || message.find("ERROR") != Ogre::String::npos || message.find("Error") != Ogre::String::npos)
	{
		InsertItem(mIndex, message.c_str());
		SetItemTextColour(mIndex++, wxColour(180, 0, 0));
		ScrollList(0, 100);
	}
	else if (message.find("warning") != Ogre::String::npos || message.find("WARNING") != Ogre::String::npos || message.find("Warning") != Ogre::String::npos)
	{
		InsertItem(mIndex, message.c_str());
		SetItemTextColour(mIndex++, wxColour(180, 180, 0));
		ScrollList(0, 100);
	}
	else if (level == Ogre::LogMessageLevel::LML_NORMAL)
	{
		InsertItem(mIndex, message.c_str());
		SetItemTextColour(mIndex++, wxColour(0, 180, 20));
		ScrollList(0, 100);
	}
}

void wxLogDisplay::OnShow(wxShowEvent& event)
{
	wxEdit::Instance().GetMainMenu()->Check(wxMainMenu_ShowLog, event.GetShow());
}