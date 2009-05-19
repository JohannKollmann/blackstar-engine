

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

class wxEditSGTGameObject : public wxPropertyGridListener
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
	SGTGameObject* GetGameObject();

	void AddGOCSection(Ogre::String name, SGTDataMap &map);
	void RemoveGOCSection(Ogre::String name);

protected:
	SGTGameObject *mGameObject;
	Ogre::String mCurrentEditPath;
	bool mFirstTimeOverideHack;

	/*
	Liefert den Namen der nächsten Kategorie, falls vorhanden, zurueck.
	*/
	Ogre::String SectionToDataMap(wxPropertyGridIterator &it, SGTDataMap *data);
};

#endif