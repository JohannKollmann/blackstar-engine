#pragma once

#include "IceIncludes.h"
#include "IceMessageListener.h"
#include "IceMessageSystem.h"

namespace Caelum
{
	class CaelumSystem;
}

namespace Ice
{

class DllExport WeatherController : public ViewMessageListener
{
private:
	Caelum::CaelumSystem *mCaelumSystem;
	double mSpeedFactor;
	bool mPaused;

	int mDay;
	int mMonth;

public:
	WeatherController(void);
	~WeatherController(void);

	Caelum::CaelumSystem* GetCaelumSystem();
	void SetSpeedFactor(double speed);
	void SetTime(int hour, int minutes);
	void ReceiveMessage(Msg &msg);
	void Update(float time);
	void SetGroundFogEnabled (bool enable);
	bool GetGroundFogEnabled ();
	void UpdateViewport();

	Ogre::Vector3 GetSunLightPosition() const;
	Ogre::ColourValue GetSunLightColour() const;
};

};