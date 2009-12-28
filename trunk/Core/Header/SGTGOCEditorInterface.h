
#pragma once

/*
Interface for Editors.
*/

#include "SGTDataMap.h"
#include "SGTIncludes.h"

class SGTGOCEditorInterface
{
public:
	virtual void CreateFromDataMap(SGTDataMap *parameters) = 0;
	virtual void GetParameters(SGTDataMap *parameters) = 0;
	virtual Ogre::String GetLabel() = 0;
	virtual void* GetUserData() { return 0; }
	virtual void InjectUserData(void* data) { }

	/*
	Attaches the editor interface to a GO and replaces the existing component width the same family, if it exists!
	*/
	virtual void AttachToGO(SGTGameObject *go) = 0;

	static SGTGOCEditorInterface* NewEditorInterfaceInstance();
};