#pragma once

#include "SGTIncludes.h"
#include "SGTGOComponent.h"

class SGTGOCGameplay : public SGTGOComponent
{
public:
	SGTGOCGameplay(void);
	virtual ~SGTGOCGameplay(void);

	goc_id_family& GetFamilyID() const { static std::string name = "GOCGameplay"; return name; }
};


class SGTGOCItem : public SGTGOCGameplay
{
public:
	SGTGOCItem(void);
	~SGTGOCItem(void);

	SGTGOComponent::goc_id_type& GetComponentID() const { static std::string name = "GOCItem"; return name; }

	void Save(SGTSaveSystem& mgr);
	void Load(SGTLoadSystem& mgr);
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "GOCItem"; *pFn = (SGTSaveableInstanceFn)&NewInstance; }
	static SGTSaveable* NewInstance() { return new SGTGOCItem; }
	std::string& TellName() { static std::string name = "GOCItem"; return name; };
};