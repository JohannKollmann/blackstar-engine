
#pragma once

#include "Ogre.h"
#include "IceIncludes.h"
#include <map>

/*Minimalistische Implementation des Ogre Compositor Frameworks. Lädt alle verfügbaren Compositor im Konstruktor
und stellt Methoden zum in- und aktivieren dieser bereit.
*/

namespace Ice
{

class CompositorLoader
{
private:
	std::map<Ogre::String, Ogre::CompositorInstance*> mCompositorInstances;
public:

	CompositorLoader();
	~CompositorLoader() { } ;

	static CompositorLoader&	Instance();

	Ogre::CompositorInstance* AddListener(Ogre::String compositorName, Ogre::CompositorInstance::Listener *listener);

	void EnableCompositor(Ogre::String compositorName);
	void DisableCompositor(Ogre::String compositorName);
};


};