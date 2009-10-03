
#pragma once

#include "wx/wx.h"
#include "wxEdit.h"
#include "wx/spinctrl.h"
#include "wx/clrpicker.h" 
#include "wx/propgrid/propgrid.h"
#include "wxPropertyGridListener.h"
#include <list>
#include "EDTIncludes.h"


class wxMaterialEditor : public wxPropertyGridListener
{
private:
	Ogre::String FindResource(Ogre::MaterialPtr material);

	Ogre::String Scan_Line_For_Material(Ogre::String line);

	struct MaterialTemplate
	{
		Ogre::String mName;
		Ogre::String mFile;
	};

	std::map<Ogre::String, MaterialTemplate> mMapTemplates;

protected:
	Ogre::MaterialPtr mCurrentMaterial;
	Ogre::String mCurrentFile;
	Ogre::String mCurrentTemplate;
	Ogre::String mCurrentTemplateFile;

public:
	wxMaterialEditor(void);
	~wxMaterialEditor(void);

	void OnApply();
	void OnActivate();
	void OnLeave();
	bool OnDropText(const wxString& text);

	void RegisterDefaultMapTemplate(Ogre::String map_type, Ogre::String tname, Ogre::String tfile);

	void SetMaterialTemplate(Ogre::String Name, Ogre::String File);
	void EditMaterial(Ogre::MaterialPtr material, bool detect_template = true);
};
