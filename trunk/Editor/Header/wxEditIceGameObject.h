

#ifndef __wxEditIceGameObject_H__
#define __wxEditIceGameObject_H__

#include "wx/wx.h"
#include "wxEdit.h"
#include "wx/spinctrl.h"
#include "wx/clrpicker.h" 
#include "wx/propgrid/propgrid.h"
#include "wxPropertyGridListener.h"
#include <list>
#include "EDTIncludes.h"
#include "IceGameObject.h"
#include "IceDataMap.h"

#include "IceGameObject.h"


/*
wxEditIceGameObject ist die Basisklasse für wxPropertyGridWindow-Listener, die die Bearbeitung eines Ice::GameObject erlauben.
Grundsätzlich wird zwischen 2 Modi unterschieden:
	-	Bearbeitung einer Resource: Hier wird ausschließlich eine Datei bearbeitet, die Ice::GameObject Eigenschaften speichert.
		Es werden dieselben Load/Save Handler wie beim laden/speichern von "richtigen" Ice::GameObject Objekten genutzt, allerdings wird
		niemals ein Ice::GameObject erstellt.
	-	Bearbeitung einer Ice::GameObject Instanz.

*/

class wxEditIceDataMap : public wxPropertyGridListener
{
public:
	wxEditIceDataMap() {};
	virtual ~wxEditIceDataMap() { };

	virtual void AddDataMapSection(Ogre::String name, Ice::DataMap &map, bool expand = false);

protected:
	/*
	Liefert den Namen der nächsten Kategorie, falls vorhanden, zurueck.
	*/
	virtual Ogre::String SectionToDataMap(wxPropertyGridIterator &it, Ice::DataMap *data);

};

class wxEditIceGameObject : public wxEditIceDataMap
{
public:
	wxEditIceGameObject();
	virtual ~wxEditIceGameObject() { };

	void SetObject(Ice::GameObject *object, bool update_ui = true);
	void SetResource(Ogre::String savepath);
	void NewResource(Ogre::String savepath, bool showcomponentbar = true);
	void OnUpdate();
	void OnApply();
	void OnLeave();
	void OnActivate();
	bool OnDropText(const wxString& text);
	Ice::GameObject* GetGameObject();

	void AddGOCSection(Ogre::String name, Ice::DataMap &map, bool expand = false);
	void RemoveGOCSection(Ogre::String name);

protected:
	Ice::GameObject *mGameObject;
	Ogre::String mCurrentEditPath;
	bool mFirstTimeOverideHack;

	Ogre::String SectionToDataMap(wxPropertyGridIterator &it, Ice::DataMap *data);
};


class wxEditIceSceneParams : public wxEditIceDataMap
{
public:
	wxEditIceSceneParams() {};
	~wxEditIceSceneParams() { };

	void OnApply();
	void OnActivate();
};

#endif