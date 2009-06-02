
#include "wxComponentBar.h"
#include "SGTSceneManager.h"
#include "wxEditSGTGameObject.h"

IMPLEMENT_CLASS(wxComponentBar, wxPanel)

BEGIN_EVENT_TABLE(wxComponentBar, wxPanel)
	EVT_CHECKBOX(-1, wxComponentBar::OnCheckBoxClicked)
END_EVENT_TABLE()

wxComponentBar::wxComponentBar(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
: wxPanel(parent, id, pos, size)
{
    //wxGridSizer* grid_sizer = new wxGridSizer(1);
	wxBoxSizer* cont_sizer = new wxBoxSizer(wxVERTICAL);
    //grid_sizer->SetHGap(5);
	int idcounter = 1;
	int maxItems = 3;
	for (std::map<Ogre::String, std::map<Ogre::String, SGTDataMap*> >::iterator i = SGTSceneManager::Instance().mGOCDefaultParameters.begin(); i != SGTSceneManager::Instance().mGOCDefaultParameters.end(); i++)
	{
		int itemCounter = 0;
		wxBoxSizer* s = new wxBoxSizer(wxHORIZONTAL);
		/*s->Add(1, 1, 1, wxEXPAND);
		wxStaticText *text = new wxStaticText(this, wxID_ANY, wxT((*i).first.c_str()));
		wxFont font = text->GetFont();
		font.SetWeight(wxBOLD);
		text->SetFont(font);
		s->Add(text);*/
		for (std::map<Ogre::String, SGTDataMap*>::iterator x = (*i).second.begin(); x != (*i).second.end(); x++)
		{
			ComponentParameters cp;
			cp.mCheckBox = new wxCheckBox(this, wxID_HIGHEST + idcounter, wxT((*x).first.c_str()));
			cp.mName = (*x).first;
			cp.mParameters = (*x).second;
			mCallbackMap.insert(std::make_pair<int, ComponentParameters>(wxID_HIGHEST + idcounter, cp));
			idcounter++;
			//cp.mCheckBox->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(wxComponentBar::OnCheckBoxClicked));
			if (itemCounter < maxItems) itemCounter++;
			else
			{
				cont_sizer->Add(s);
				s = new wxBoxSizer(wxHORIZONTAL);
				itemCounter = 0;
			}
			s->Add(cp.mCheckBox);
		}
		//s->Add(1, 1, 1, wxEXPAND);
		cont_sizer->Add(s);
	}
    //cont_sizer->Add(grid_sizer, 1, wxEXPAND | wxALL, 5);
    SetSizer(cont_sizer);
    GetSizer()->SetSizeHints(this);
}

wxComponentBar::~wxComponentBar(void)
{
}

void wxComponentBar::OnCheckBoxClicked(wxCommandEvent& event)
{
	if (wxEdit::Instance().GetpropertyWindow()->GetCurrentPageName() == "EditGameObject")
	{
		int id = event.GetId();
		std::map<int, ComponentParameters>::iterator i = mCallbackMap.find(id);
		if (i != mCallbackMap.end())
		{
			if ((*i).second.mCheckBox->IsChecked())
			{
				((wxEditSGTGameObject*)(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage()))->AddGOCSection((*i).second.mName, *(*i).second.mParameters);
			}
			else
			{
				((wxEditSGTGameObject*)(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage()))->RemoveGOCSection((*i).second.mName);
			}
		}
	}
}

void wxComponentBar::ResetCheckBoxes()
{
	for (std::map<int, ComponentParameters>::iterator x = mCallbackMap.begin(); x != mCallbackMap.end(); x++)
	{
		(*x).second.mCheckBox->Set3StateValue(wxCheckBoxState::wxCHK_UNCHECKED);
	}
}

void wxComponentBar::SetSections(std::list<ComponentSection> sections)
{
	ResetCheckBoxes();
	for (std::list<ComponentSection>::iterator i = sections.begin(); i != sections.end(); i++)
	{
		for (std::map<int, ComponentParameters>::iterator x = mCallbackMap.begin(); x != mCallbackMap.end(); x++)
		{
			if ((*x).second.mName == (*i).mSectionName)
			{
				(*x).second.mCheckBox->Set3StateValue(wxCheckBoxState::wxCHK_CHECKED);
				break;
			}
		}
	}
}