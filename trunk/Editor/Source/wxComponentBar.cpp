
#include "wxComponentBar.h"
#include "IceSceneManager.h"
#include "propGridEditIceEditorInterface.h"
#include "wxEdit.h"

IMPLEMENT_CLASS(wxComponentBar, wxPanel)

BEGIN_EVENT_TABLE(wxComponentBar, wxPanel)
	EVT_CHECKBOX(-1, wxComponentBar::OnCheckBoxClicked)
END_EVENT_TABLE()

wxComponentBar::wxComponentBar(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
: wxPanel(parent, id, pos, size)
{
    //wxGridSizer* grid_sizer = new wxGridSizer(1);
	wxBoxSizer* cont_sizer = new wxBoxSizer(wxVERTICAL);
	SetMaxSize(wxSize(GetSize().GetWidth(), 200));
	int idcounter = 1;
	for (std::map<Ogre::String, std::map<Ogre::String, Ice::DataMap> >::iterator i = Ice::SceneManager::Instance().mGOCDefaultParameters.begin(); i != Ice::SceneManager::Instance().mGOCDefaultParameters.end(); i++)
	{
		wxStaticBox *box = new wxStaticBox(this, wxID_ANY, _T(""));
		wxSizer *s = new wxStaticBoxSizer(box, wxHORIZONTAL);
		//wxBoxSizer* s = new wxBoxSizer(wxHORIZONTAL);
		s->SetSizeHints(this);
		for (std::map<Ogre::String, Ice::DataMap>::iterator x = (*i).second.begin(); x != (*i).second.end(); x++)
		{
			ComponentParameters cp;
			cp.mCheckBox = new wxCheckBox(this, wxID_HIGHEST + idcounter, wxT((*x).first.c_str()));
			cp.mName = (*x).first;
			cp.mFamily = (*i).first;
			cp.mParameters = &(*x).second;
			mCallbackMap.insert(std::make_pair<int, ComponentParameters>(wxID_HIGHEST + idcounter, cp));
			idcounter++;
			s->Add(cp.mCheckBox);
		}
		//boxSizer->Add(s);
		cont_sizer->Add(s);//, wxSizerFlags().Border());
	}
    SetSizer(cont_sizer);
	cont_sizer->SetSizeHints(this);
}

wxComponentBar::~wxComponentBar(void)
{
}

void wxComponentBar::OnCheckBoxClicked(wxCommandEvent& event)
{
	wxEditGOCSections *sections = dynamic_cast<wxEditGOCSections*>(wxEdit::Instance().GetpropertyWindow()->GetCurrentPage());
	if (sections)
	{
		int id = event.GetId();
		std::map<int, ComponentParameters>::iterator i = mCallbackMap.find(id);
		if (i != mCallbackMap.end())
		{
			if ((*i).second.mCheckBox->IsChecked())
			{
				sections->AddGOCSection((*i).second.mName, *(*i).second.mParameters);
			}
			else
			{
				sections->RemoveGOCSection((*i).second.mName);
			}
			NotifyGroupCheck((*i).second.mCheckBox->IsChecked(), (*i).second.mName, (*i).second.mFamily);
		}
	}
}

void wxComponentBar::ResetCheckBoxes()
{
	for (std::map<int, ComponentParameters>::iterator x = mCallbackMap.begin(); x != mCallbackMap.end(); x++)
	{
		(*x).second.mCheckBox->Set3StateValue(wxCheckBoxState::wxCHK_UNCHECKED);
		NotifyGroupCheck(false, (*x).second.mName, (*x).second.mFamily);
	}
}

void wxComponentBar::NotifyGroupCheck(bool checked, Ogre::String checked_name, Ogre::String group)
{
	if (group.find("_x") != Ogre::String::npos)		//if it is an exclusive group
	{
		for (std::map<int, ComponentParameters>::iterator x = mCallbackMap.begin(); x != mCallbackMap.end(); x++)
		{
			if ((*x).second.mFamily == group && (*x).second.mName != checked_name)
				(*x).second.mCheckBox->Enable(!checked);
		}
	}
}

void wxComponentBar::SetSections(std::vector<ComponentSection> &sections)
{
	ResetCheckBoxes();
	for (auto i = sections.begin(); i != sections.end(); i++)
	{
		for (std::map<int, ComponentParameters>::iterator x = mCallbackMap.begin(); x != mCallbackMap.end(); x++)
		{
			if ((*x).second.mName == (*i).mSectionName)
			{
				(*x).second.mCheckBox->Set3StateValue(wxCheckBoxState::wxCHK_CHECKED);
				NotifyGroupCheck(true, (*x).second.mName, (*x).second.mFamily);
				break;
			}
		}
	}
}

void wxComponentBar::SetSectionStatus(Ogre::String name, bool checked)
{
	for (std::map<int, ComponentParameters>::iterator x = mCallbackMap.begin(); x != mCallbackMap.end(); x++)
	{
		if ((*x).second.mName == name)
		{
			if (checked) (*x).second.mCheckBox->Set3StateValue(wxCheckBoxState::wxCHK_CHECKED);
			else (*x).second.mCheckBox->Set3StateValue(wxCheckBoxState::wxCHK_UNCHECKED);
			return;
		}
	}
}