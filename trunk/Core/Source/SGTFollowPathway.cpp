
#include "SGTFollowPathway.h"
#include "SGTPathfinder.h"
#include "SGTGOCAI.h"
#include "SGTGOCCharacterController.h"

SGTFollowPathway::SGTFollowPathway(SGTGOCAI *ai, Ogre::String targetWP, float radius)
{
	mAIObject = ai;
	mRadius = radius;
	mTargetWP = targetWP;
};

SGTFollowPathway::~SGTFollowPathway()
{
	mPath.clear();
};

void SGTFollowPathway::OnEnter()
{
	SGTPathfinder::Instance().FindPath(mAIObject->GetOwner()->GetGlobalPosition(), mTargetWP, &mPath);
	mCurrentTarget = mPath.begin();
}

bool SGTFollowPathway::OnUpdate(float time)
{
	Ogre::Vector3 currPos = mAIObject->GetOwner()->GetGlobalPosition();
	float dist = currPos.distance(*mCurrentTarget);
	if (dist < mRadius)
	{
		mCurrentTarget++;
	}
	if (mCurrentTarget == mPath.end())
	{
		Ogre::LogManager::getSingleton().logMessage("Ziel erreicht!");
		mAIObject->BroadcastMovementState(0);
		return true;
	}

	Ogre::Vector3 direction = (*mCurrentTarget)-currPos;
	direction.y = 0;
	Ogre::Quaternion quat = Ogre::Vector3::UNIT_Z.getRotationTo(direction);
	mAIObject->GetOwner()->SetGlobalOrientation(quat);
	int movementstate = SGTCharacterMovement::FORWARD;
	mAIObject->BroadcastMovementState(movementstate);
	return false;
}

void SGTFollowPathway::smoothPath()
{
	Ogre::Vector3 wp1, wp2, wp3, strecke1, strecke2; //wp2 ist der wp an dem gesmooth wird, wp1 und wp3 wird nur fürs berechnen der strecken zwischen den wps gebraucht
    std::vector<Ogre::Vector3> newWPList;
    Ogre::LogManager::getSingleton().logMessage(Ogre::String("Initialised"));
    newWPList.push_back (mPath[0]);
    for(unsigned int i = 1; i < mPath.size()-1; ++i)
    {
        Ogre::LogManager::getSingleton().logMessage(Ogre::String("neuer Durchgang:" + Ogre::StringConverter::toString(i)));
        wp1 = mPath[i-1];
        wp2 = mPath[i];
        wp3 = mPath[i+1];
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
    int end = mPath.size();
    newWPList.push_back(mPath[end-1]);
	mPath.clear();
	for(unsigned int i = 0; i<newWPList.size(); ++i)
	{
		mPath.push_back(newWPList[i]);
	}
    Ogre::LogManager::getSingleton().logMessage(Ogre::String("einfügen des letzen wps"));
    //mPath = newWPList;
}