#ifndef __SGTGUISystem_H__
#define __SGTGUISystem_H__

#include "Ogre.h"
#include "SGTMessageListener.h"
#include "SGTMessageSystem.h"
#include "SGTIncludes.h"
#include "SGTMain.h"

class SGTDllExport SGTGUISystem : public SGTMessageListener
{
public:
	SGTGUISystem(void);
	~SGTGUISystem(void);

	void ReceiveMessage(SGTMsg &msg);
private:
	float m_fXPos, m_fYPos;
	static const float m_fFactor;
	bool m_bMeshInitialized;
};

#endif