#pragma once

#ifndef ResourceGroupReloader_H
#define ResourceGroupReloader_H

#include <utility>
#include <string>
#include <vector>
#include <OgreRenderable.h>
#include <OgreLog.h>
///\brief a fake class with different useful methods for manipulating Resourcegroups.
/// please note that it was not tested with background loading.

class ResourceGroupReloader
{
private:

	/// \brief anti copy constructor (no use)
	ResourceGroupReloader(const ResourceGroupReloader&);
	/// \brief anti affector (no use)
	ResourceGroupReloader& operator=(const ResourceGroupReloader&);

	/// \brief a helper method to visit all overlay
	/// it uses a recursive call
	void visitRecursivelyRenderablesFrom(Ogre::OverlayContainer* pOverlayContainer, Ogre::Renderable::Visitor& pVisitor, bool debugRenderable = false);

	// ------ helper classes

	/// \brief this visitor will be used to set the material on known renderable that allow this operation.
	/// for other user class renderable, must be tweaked/changed
	class UpdateMaterialRenderableVisitor : public Ogre::Renderable::Visitor
	{
	private:
		UpdateMaterialRenderableVisitor(const UpdateMaterialRenderableVisitor&);///<\brief anti copyconstructor
		UpdateMaterialRenderableVisitor& operator=(const UpdateMaterialRenderableVisitor&);///<\brief anti affector
	public: 
		/// \brief default constructor
		UpdateMaterialRenderableVisitor();
		/// \brief called for each renderable
		virtual void visit(Ogre::Renderable *rend, Ogre::ushort lodIndex, bool isDebug, Ogre::Any *pAny=0); 
	};

public:
	/// \brief default constructor
	ResourceGroupReloader(void);

	/// \brief destructor
	~ResourceGroupReloader(void);

	/// \brief this serves the same purpose than reloadAResourceGroup, but it does not destroy/recreate the ResourceGroup.
	/// \param the name of the resourcegroup
	/// on the one hand, you don't need to worry about the path and order of the locations.
	/// on the other hand, there is no test that the resources were really cleared and reloaded.
	/// personnally I prefer this thought, because it's much smarter & less costly in the end.
	bool reloadAResourceGroupWithoutDestroyingIt(const std::string& pResourceGroupName);

	/// \brief return true if the resourcegroup exists
	/// \param the name of the resourcegroup
	bool resourceGroupExist(const std::string& pResourceGroupName);

	/// \brief updating informations about materials on all 'reachable' renderables
	/// that are currently used by the different scenemanagers and overlays
	void updateOnEveryRenderable();

	void reloadResourceGroup(const std::string& resourceGroupName);
};

#endif

