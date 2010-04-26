

#ifndef __wxEditSGTGameObject_H__
#define __wxEditSGTGameObject_H__

#include "wx/wx.h"
#include "wxEdit.h"
#include "wx/spinctrl.h"
#include "wx/clrpicker.h" 
#include "wx/propgrid/propgrid.h"
#include "wxPropertyGridListener.h"
#include <list>
#include "EDTIncludes.h"
#include "SGTGameObject.h"
#include "SGTDataMap.h"

#include "SGTGameObject.h"


/*
wxEditSGTGameObject ist die Basisklasse für wxPropertyGridWindow-Listener, die die Bearbeitung eines SGTGameObject erlauben.
Grundsätzlich wird zwischen 2 Modi unterschieden:
	-	Bearbeitung einer Resource: Hier wird ausschließlich eine Datei bearbeitet, die SGTGameObject Eigenschaften speichert.
		Es werden dieselben Load/Save Handler wie beim laden/speichern von "richtigen" SGTGameObject Objekten genutzt, allerdings wird
		niemals ein SGTGameObject erstellt.
	-	Bearbeitung einer SGTGameObject Instanz.

*/

class wxEditSGTDataMap : public wxPropertyGridListener
{
public:
	wxEditSGTDataMap() {};
	virtual ~wxEditSGTDataMap() { };

	virtual void AddDataMapSection(Ogre::String name, SGTDataMap &map, bool expand = false);

protected:
	/*
	Liefert den Namen der nächsten Kategorie, falls vorhanden, zurueck.
	*/
	virtual Ogre::String SectionToDataMap(wxPropertyGridIterator &it, SGTDataMap *data);

};

class wxEditSGTGameObject : public wxEditSGTDataMap
{
public:
	wxEditSGTGameObject();
	virtual ~wxEditSGTGameObject() { };

	void SetObject(SGTGameObject *object, bool update_ui = true);
	void SetResource(Ogre::String savepath);
	void NewResource(Ogre::String savepath, bool showcomponentbar = true);
	void OnUpdate();
	void OnApply();
	void OnLeave();
	void OnActivate();
	bool OnDropText(const wxString& text);
	SGTGameObject* GetGameObject();

	void AddGOCSection(Ogre::String name, SGTDataMap &map, bool expand = false);
	void RemoveGOCSection(Ogre::String name);

protected:
	SGTGameObject *mGameObject;
	Ogre::String mCurrentEditPath;
	bool mFirstTimeOverideHack;

	Ogre::String SectionToDataMap(wxPropertyGridIterator &it, SGTDataMap *data);
};


class wxEditSGTSceneParams : public wxEditSGTDataMap
{
public:
	wxEditSGTSceneParams() {};
	~wxEditSGTSceneParams() { };

	void OnApply();
	void OnActivate();
};

#endif