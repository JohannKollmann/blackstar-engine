#pragma once

#include "IceIncludes.h"
#include "IceGOComponent.h"

namespace Ice
{

class GOCGameplay : public GOComponent
{
public:
	GOCGameplay(void);
	virtual ~GOCGameplay(void);

	goc_id_family& GetFamilyID() const { static std::string name = "Gameplay"; return name; }
};


class GOCItem : public GOCGameplay
{
public:
	GOCItem(void);
	~GOCItem(void);

	GOComponent::goc_id_type& GetComponentID() const { static std::string name = "Item"; return name; }

	void Save(LoadSave::SaveSystem& mgr);
	void Load(LoadSave::LoadSystem& mgr);
	static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "Item"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; }
	static LoadSave::Saveable* NewInstance() { return new GOCItem; }
	std::string& TellName() { static std::string name = "Item"; return name; };
};

};