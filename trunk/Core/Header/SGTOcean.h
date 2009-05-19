#pragma once

#include "Hydrax.h"
#include "Noise/Perlin/Perlin.h"
#include "Modules/ProjectedGrid/ProjectedGrid.h"
#include "SGTMessageSystem.h"	
#include "SGTIncludes.h"

class SGTDllExport SGTOcean : public SGTMessageListener
{
private:
	Hydrax::Hydrax *mHydrax;
	Hydrax::Module::ProjectedGrid *mGeometry;

public:
	SGTOcean(void);
	~SGTOcean(void);

	void ReceiveMessage(SGTMsg &msg);
};
