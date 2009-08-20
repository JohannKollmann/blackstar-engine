
#pragma once

/*
Interface for Editors.
*/

#include "SGTDataMap.h"
#include "SGTIncludes.h"

class SGTGOCEditorInterface
{
public:
	//SGTGOCEditorInterface() //virtual ~SGTGOCEditorInterface() {};
	virtual void CreateFromDataMap(SGTDataMap *parameters) = 0;
	virtual void GetParameters(SGTDataMap *parameters) = 0;
	virtual void* GetUserData() { return 0; }
	virtual void InjectUserData(void* data) { }
	virtual bool IsViewComponent() = 0;
	static SGTGOCEditorInterface* NewEditorInterfaceInstance();
};