#pragma once

#include "Caelum.h"
#include "IceIncludes.h"
#include "IceMessageListener.h"
#include "IceMessageSystem.h"

namespace Ice
{

class DllExport WeatherController : public MessageListener
{
private:
	Caelum::CaelumSystem *mCaelumSystem;
	double mSpeedFactor;
	bool mPaused;

	int mOneTime;

public:
	WeatherController(void);
	~WeatherController(void);

	Caelum::CaelumSystem* GetCaelumSystem();
	void SetSpeedFactor(double speed);
	void ReceiveMessage(Msg &msg);
	void Update(float time);
	void SetGroundFogEnabled (bool enable);
	bool GetGroundFogEnabled ();
	void UpdateViewport();
};

};