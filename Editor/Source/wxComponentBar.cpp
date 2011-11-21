
#include "wxComponentBar.h"
#include "IceComponentFactory.h"
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
	for (std::map<Ogre::String, std::map<Ogre::String, Ice::ComponentFactory::GOCDefaultParams> >::iterator i = Ice::ComponentFactory::Instance().GetDefaultParametersIteratorBegin(); i != Ice::ComponentFactory::Instance().GetDefaultParametersIteratorEnd(); i++)
	{
		wxStaticBox *box = new wxStaticBox(this, wxID_ANY, "");
		wxSizer *s = new wxStaticBoxSizer(box, wxHORIZONTAL);
		//wxBoxSizer* s = new wxBoxSizer(wxHORIZONTAL);
		s->SetSizeHints(this);
		for (std::map<Ogre::String, Ice::ComponentFactory::GOCDefaultParams>::iterator x = (*i).second.begin(); x != (*i).second.end(); x++)
		{
			ComponentParameters cp;
			cp.checkBox = new wxCheckBox(this, wxID_HIGHEST + idcounter, ((*x).first.c_str()));
			cp.exclusiveFamily = (*x).second.exclusiveFamily;
			cp.gocType = (*x).first;
			cp.row = (*i).first;
			cp.parameters = &(*x).second.params;
			mCallbackMap.insert(std::make_pair<int, ComponentParameters>(wxID_HIGHEST + idcounter, cp));
			idcounter++;
			s->Add(cp.checkBox);
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
			if ((*i).second.checkBox->IsChecked())
			{
				sections->AddGOCSection((*i).second.gocType, *(*i).second.parameters);
			}
			else
			{
				sections->RemoveGOCSection((*i).second.gocType);
			}
			NotifyGroupCheck((*i).second.checkBox->IsChecked(), (*i).second.gocType, (*i).second.row);
		}
	}
}

void wxComponentBar::ResetCheckBoxes()
{
	for (std::map<int, ComponentParameters>::iterator x = mCallbackMap.begin(); x != mCallbackMap.end(); x++)
	{
		(*x).second.checkBox->Set3StateValue(wxCheckBoxState::wxCHK_UNCHECKED);
		NotifyGroupCheck(false, (*x).second.gocType, (*x).second.row);
	}
}

void wxComponentBar::NotifyGroupCheck(bool checked, Ogre::String checked_name, Ogre::String group)
{
	Ogre::String exclusiveFamily = "";
	for (std::map<int, ComponentParameters>::iterator x = mCallbackMap.begin(); x != mCallbackMap.end(); x++)
	{
		if ((*x).second.row == group && (*x).second.gocType == checked_name)
			exclusiveFamily = (*x).second.exclusiveFamily;
	}

	if (exclusiveFamily != "")		//if it is an exclusive group
	{
		for (std::map<int, ComponentParameters>::iterator x = mCallbackMap.begin(); x != mCallbackMap.end(); x++)
		{
			if ((*x).second.exclusiveFamily == exclusiveFamily && (*x).second.gocType != checked_name)
				(*x).second.checkBox->Enable(!checked);
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
			if ((*x).second.gocType == (*i).mSectionName)
			{
				(*x).second.checkBox->Set3StateValue(wxCheckBoxState::wxCHK_CHECKED);
				NotifyGroupCheck(true, (*x).second.gocType, (*x).second.row);
				break;
			}
		}
	}
}

void wxComponentBar::SetSections(std::vector<ComponentSectionPtr> &sections)
{
	ResetCheckBoxes();
	for (auto i = sections.begin(); i != sections.end(); i++)
	{
		for (std::map<int, ComponentParameters>::iterator x = mCallbackMap.begin(); x != mCallbackMap.end(); x++)
		{
			if ((*x).second.gocType == (*i)->mSectionName)
			{
				(*x).second.checkBox->Set3StateValue(wxCheckBoxState::wxCHK_CHECKED);
				NotifyGroupCheck(true, (*x).second.gocType, (*x).second.row);
				break;
			}
		}
	}
}

void wxComponentBar::SetSectionStatus(Ogre::String name, bool checked)
{
	for (std::map<int, ComponentParameters>::iterator x = mCallbackMap.begin(); x != mCallbackMap.end(); x++)
	{
		if ((*x).second.gocType == name)
		{
			if (checked) (*x).second.checkBox->Set3StateValue(wxCheckBoxState::wxCHK_CHECKED);
			else (*x).second.checkBox->Set3StateValue(wxCheckBoxState::wxCHK_UNCHECKED);
			return;
		}
	}
}