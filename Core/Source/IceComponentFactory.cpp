
#include "IceComponentFactory.h"
#include "IceGOCPhysics.h"
#include "IceGOCView.h"
#include "IceGOCPlayerInput.h"
#include "IceGOCCameraController.h"
#include "IceGOCAnimatedCharacter.h"
#include "IceGOCWaypoint.h"
#include "IceGOCAI.h"
#include "IceGOCMover.h"
#include "IceGOCScript.h"
#include "IceGOCForceField.h"
#include "IceGOCJoint.h"

namespace Ice
{
	ComponentFactory::ComponentFactory()
	{
		LoadSave::LoadSave::Instance().RegisterObject(&GOCWaypoint::Register);

		LoadSave::LoadSave::Instance().RegisterObject(&GOCMeshRenderable::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GOCPfxRenderable::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GOCLocalLightRenderable::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GOCSound3D::Register);

		LoadSave::LoadSave::Instance().RegisterObject(&GOCRigidBody::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GOCDestructible::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GOCStaticBody::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GOCTrigger::Register);

		LoadSave::LoadSave::Instance().RegisterObject(&GOCAnimatedCharacter::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GOCCharacterController::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GOCPlayerInput::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GOCCameraController::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GOCSimpleCameraController::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GOCScriptedProperties::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GOCScript::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GOCScriptMessageCallback::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GOCAI::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GOCForceField::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GOCBillboard::Register);

		LoadSave::LoadSave::Instance().RegisterObject(&GOCMover::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GOCAnimKey::Register);

		LoadSave::LoadSave::Instance().RegisterObject(&GOCFixedJoint::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GOCParticleChain::Register);

		RegisterGOCPrototype(GOCEditorInterfacePtr(new GOCMeshRenderable()), "A");
		RegisterGOCPrototype(GOCEditorInterfacePtr(new GOCPfxRenderable()), "A");
		RegisterGOCPrototype(GOCEditorInterfacePtr(new GOCLocalLightRenderable()), "A");
		RegisterGOCPrototype(GOCEditorInterfacePtr(new GOCSound3D()), "A");
		RegisterGOCPrototype(GOCEditorInterfacePtr(new GOCAnimatedCharacter()), "A");
		RegisterGOCPrototype(GOCEditorInterfacePtr(new GOCBillboard()), "A");

		RegisterGOCPrototype(GOCEditorInterfacePtr(new GOCRigidBody()), "B", "B");
		RegisterGOCPrototype(GOCEditorInterfacePtr(new GOCDestructible()), "F", "F");
		RegisterGOCPrototype(GOCEditorInterfacePtr(new GOCStaticBody()), "B", "B");
		RegisterGOCPrototype(GOCEditorInterfacePtr(new GOCCharacterController()), "B", "B");
		RegisterGOCPrototype(GOCEditorInterfacePtr(new GOCTrigger()), "B", "B");

		RegisterGOCPrototype(GOCEditorInterfacePtr(new GOCAI()), "C", "C");
		RegisterGOCPrototype(GOCEditorInterfacePtr(new GOCPlayerInput()), "C", "C");
		RegisterGOCPrototype(GOCEditorInterfacePtr(new GOCWaypoint()), "C");

		RegisterGOCPrototype(GOCEditorInterfacePtr(new GOCSimpleCameraController()), "D", "D");
		RegisterGOCPrototype(GOCEditorInterfacePtr(new GOCCameraController()), "D", "D");

		RegisterGOCPrototype(GOCEditorInterfacePtr(new GOCMover()), "E");
		RegisterGOCPrototype(GOCEditorInterfacePtr(new GOCScript()), "E");
		RegisterGOCPrototype(GOCEditorInterfacePtr(new GOCForceField()), "E");
		RegisterGOCPrototype(GOCEditorInterfacePtr(new GOCAnimKey()));

		RegisterGOCPrototype(GOCEditorInterfacePtr(new GOCFixedJoint()), "F", "F");
		RegisterGOCPrototype(GOCEditorInterfacePtr(new GOCParticleChain()), "F", "F");
	}

	ComponentFactory::~ComponentFactory()
	{
		Shutdown();
	}

	GOCEditorInterface* ComponentFactory::GetGOCPrototype(const Ogre::String &type)
	{
		auto i = mGOCPrototypes.find(type);
		if (i != mGOCPrototypes.end())
			return i->second.get();
		return nullptr;
	}

	GOCEditorInterface* ComponentFactory::CreateGOC(const Ogre::String &type)
	{
		auto i = mGOCPrototypes.find(type);
		if (i != mGOCPrototypes.end())
		{
			GOCEditorInterface *goc = (*i).second->New();
			return goc;
		}
		IceAssert(false);
		return nullptr;
	}

	void ComponentFactory::RegisterComponentDefaultParams(const Ogre::String &type, DataMap &params, const Ogre::String &editorRow, const Ogre::String &exclusiveFamily)
	{
		GOCDefaultParams gocParams;
		gocParams.exclusiveFamily = exclusiveFamily;
		gocParams.params = params;
		auto i = mGOCDefaultParameters.find(editorRow);
		if (i == mGOCDefaultParameters.end())
		{
			std::map<Ogre::String, GOCDefaultParams> types;
			types.insert(std::make_pair<Ogre::String, GOCDefaultParams>(type, gocParams));
			mGOCDefaultParameters.insert(std::make_pair<Ogre::String, std::map<Ogre::String, GOCDefaultParams> >(editorRow, types));
		}
		else i->second.insert(std::make_pair<Ogre::String, GOCDefaultParams>(type, gocParams));
	}

	void ComponentFactory::RegisterGOCPrototype(GOCEditorInterfacePtr prototype)
	{
		mGOCPrototypes.insert(std::make_pair(prototype->GetLabel(), prototype));
	}
	void ComponentFactory::RegisterGOCPrototype(GOCEditorInterfacePtr prototype, const Ogre::String &editorRow, const Ogre::String &exclusiveFamily)
	{
		mGOCPrototypes.insert(std::make_pair(prototype->GetLabel(), prototype));
		DataMap params;
		prototype->GetDefaultParameters(&params);
		RegisterComponentDefaultParams(prototype->GetLabel(), params, editorRow, exclusiveFamily);
	}

	ComponentFactory& ComponentFactory::Instance()
	{
		static ComponentFactory theOneAndOnly;
		return theOneAndOnly;
	};
}