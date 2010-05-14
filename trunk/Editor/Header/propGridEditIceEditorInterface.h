

#pragma once

#include "wxPropertyGridListener.h"
#include "EDTIncludes.h"
#include "IceDataMap.h"


/*
wxEditIceGameObject ist die Basisklasse f�r wxPropertyGridWindow-Listener, die die Bearbeitung eines Ice::GameObject erlauben.
Grunds�tzlich wird zwischen 2 Modi unterschieden:
	-	Bearbeitung einer Resource: Hier wird ausschlie�lich eine Datei bearbeitet, die Ice::GameObject Eigenschaften speichert.
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
	Liefert den Namen der n�chsten Kategorie, falls vorhanden, zurueck.
	*/
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