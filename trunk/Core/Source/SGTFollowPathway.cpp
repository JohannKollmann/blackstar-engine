
#include "SGTFollowPathway.h"
#include "SGTGOCAI.h"

SGTFollowPathway::SGTFollowPathway(Ogre::String targetWP)
{
};

SGTFollowPathway::~SGTFollowPathway()
{
	mWPList.clear();
};

bool SGTFollowPathway::OnUpdate(float time)
{
	if (mAIObject->GetOwner()->GetGlobalPosition().distance(*mCurrentTarget) < mRadius)
	{
		mCurrentTarget++;
	}
	if (mCurrentTarget == mWPList.end())
	{
		Ogre::LogManager::getSingleton().logMessage("Ziel erreicht!");
		return true;
	}
	Ogre::Vector3 steerTarget = *mCurrentTarget;
	Ogre::Quaternion q;

	return false;
};

/*void SGTPathway::smoothPath()
{
	Ogre::Vector3 wp1, wp2, wp3, strecke1, strecke2; //wp2 ist der wp an dem gesmooth wird, wp1 und wp3 wird nur fürs berechnen der strecken zwischen den wps gebraucht
    std::vector<Ogre::Vector3> newWPList;
    Ogre::LogManager::getSingleton().logMessage(Ogre::String("Initialised"));
    newWPList.push_back (mWPList[0]);
    for(unsigned int i = 1; i < mWPList.size()-1; ++i)
    {
        Ogre::LogManager::getSingleton().logMessage(Ogre::String("neuer Durchgang:" + Ogre::StringConverter::toString(i)));
        wp1 = mWPList[i-1];
        wp2 = mWPList[i];
        wp3 = mWPList[i+1];
		strecke1 = wp1 - wp2;
		strecke1 = strecke1.normalise();
		strecke2 = wp3 - wp2;
		strecke2 = strecke2.normalise();
        Ogre::Vector3 point;
        Ogre::LogManager::getSingleton().logMessage(Ogre::String("Vektoren berechnet"));
        for (float k = 0.0; k < 1.1; k += 0.2)
        {
        
            point =    ( ( (1-k)*(1-k) ) * strecke1 + (k*k) * strecke2 ) + wp2;
            newWPList.push_back(point);
        }
        Ogre::LogManager::getSingleton().logMessage(Ogre::String("Kurve berechnet"));
    }
    Ogre::LogManager::getSingleton().logMessage(Ogre::String("Smoothen beendet"));
    int end = mWPList.size();
    newWPList.push_back(mWPList[end-1]);
	mWPList.clear();
	for(unsigned int i = 0; i<newWPList.size(); ++i)
	{
		mWPList.push_back(newWPList[i]);
	}
    Ogre::LogManager::getSingleton().logMessage(Ogre::String("einfügen des letzen wps"));
    //mWPList = newWPList;
}*/