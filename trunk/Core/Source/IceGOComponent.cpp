
#include "IceGOComponent.h"
#include "IceGameObject.h"

namespace Ice
{

void GOComponent::SetOwner(GameObject *go)
{
	mOwnerGO = go;
	UpdatePosition(go->GetGlobalPosition());
	UpdateOrientation(go->GetGlobalOrientation());
	UpdateScale(go->GetGlobalScale());
}

};