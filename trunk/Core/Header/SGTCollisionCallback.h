
#pragma once

#include "NxOgre.h"

class SGTCollisionCallback
{
public:
	SGTCollisionCallback(void);
	~SGTCollisionCallback(void);

	void onStartTouch(NxOgre::Actor*, NxOgre::Actor*, NxOgre::ContactStream*);
	void onEndTouch(NxOgre::Actor*, NxOgre::Actor*, NxOgre::ContactStream*);
	void onTouch(NxOgre::Actor*, NxOgre::Actor*, NxOgre::ContactStream*);
};