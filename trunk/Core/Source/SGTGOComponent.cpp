
#include "SGTGOComponent.h"
#include "SGTGameObject.h"

void SGTGOComponent::SetOwner(SGTGameObject *go)
{
	mOwnerGO = go;
	UpdatePosition(go->GetGlobalPosition());
	UpdateOrientation(go->GetGlobalOrientation());
	UpdateScale(go->GetGlobalScale());
}