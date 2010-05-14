
#ifndef _AXIS_OBJECT_H_
#define _AXIS_OBJECT_H_

#include "IceGOComponent.h"
#include "IceGOCOgreNode.h"
#include "EDTIncludes.h"

class AxisComponent : public Ice::GOCOgreNode
{
	enum BoxParts
	{
		BOX_NONE	= 0x00,
		BOX_TOP		= 0x01,
		BOX_BOT		= 0x02,
		BOX_FRONT	= 0x04,
		BOX_BACK	= 0x08,
		BOX_LEFT	= 0x10,
		BOX_RIGHT	= 0x20,
		BOX_ALL		= 0xFF
	};
private:
	Ogre::ManualObject* mAxisObject;

	void	addMaterial(const Ogre::String& mat, Ogre::ColourValue &clr, Ogre::SceneBlendType sbt);
	void	addBox(Ogre::ManualObject* obj, Ogre::Vector3 dim, Ogre::Vector3 pos, Ogre::ColourValue color, short boxMask);
	Ogre::ManualObject* createAxis(const Ogre::String &name, Ogre::Vector3 scale);

public:
	AxisComponent();
	~AxisComponent();
	goc_id_type& GetComponentID() const { static std::string name = "AxisObject"; return name; } 
	goc_id_family& GetFamilyID() const { static std::string name = "AxisObject"; return name; } 
	void SetOwner(Ice::GameObject *go);

	void Save(LoadSave::SaveSystem& mgr) {};
	void Load(LoadSave::LoadSystem& mgr) {};
	virtual std::string& TellName() { static std::string name = "AxisObject"; return name; };
	static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "AxisObject"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
	static LoadSave::Saveable* NewInstance() { return new AxisComponent; };
};

#endif //--_AXIS_OBJECT_H_
