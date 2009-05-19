
#ifndef __SGTCompositorLoader_H__
#define __SGTCompositorLoader_H__

#include "Ogre.h"
#include "SGTIncludes.h"
#include <map>

/*Minimalistische Implementation des Ogre Compositor Frameworks. Lädt alle verfügbaren Compositor im Konstruktor
und stellt Methoden zum in- und aktivieren dieser bereit.
*/

class SGTCompositorLoader
{
private:
	std::map<Ogre::String, Ogre::CompositorInstance*> mCompositorInstances;
public:

	SGTCompositorLoader();
	~SGTCompositorLoader() { } ;

	static SGTCompositorLoader&	Instance();

	Ogre::CompositorInstance* AddListener(Ogre::String compositorName, Ogre::CompositorInstance::Listener *listener);

	void EnableCompositor(Ogre::String compositorName);
	void DisableCompositor(Ogre::String compositorName);
};


#endif