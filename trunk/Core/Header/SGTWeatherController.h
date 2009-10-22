#ifndef __SGTWeatherController_H__
#define __SGTWeatherController_H__

#include "Caelum.h"
#include "SGTIncludes.h"
#include "SGTMessageListener.h"
#include "SGTMessageSystem.h"

class SGTDllExport SGTWeatherController : public SGTMessageListener
{
private:
	Caelum::CaelumSystem *mCaelumSystem;
	double mSpeedFactor;
	bool mPaused;

	int mOneTime;

public:
	SGTWeatherController(void);
	~SGTWeatherController(void);

	Caelum::CaelumSystem* GetCaelumSystem();
	void SetSpeedFactor(double speed);
	void ReceiveMessage(SGTMsg &msg);
	void Update(float time);
	void SetGroundFogEnabled (bool enable);
	bool GetGroundFogEnabled ();
	void UpdateViewport();
};

#endif