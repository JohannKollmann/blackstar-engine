
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
	Ogre::String ScanPath(Ogre::String path, Ogre::String filename);

	Ogre::String Scan_Line_For_Material(Ogre::String line);

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

	void SetMaterialTemplate(Ogre::String Name, Ogre::String File);
	void EditMaterial(Ogre::MaterialPtr material, bool detect_template = true);
};
