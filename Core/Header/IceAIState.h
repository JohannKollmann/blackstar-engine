
#pragma once

#include "IceIncludes.h"
#include <functional>

namespace Ice
{

class AIState
{
public:
	struct PointerCompare_Less
		: std::binary_function<const AIState*, const AIState*, bool>
	{
		bool operator() (const AIState* lhs, const AIState* rhs ) const
		{
			if (lhs == 0) return false;
			if (rhs == 0) return false;
			return (*lhs < *rhs);
		}
	};

protected:
	GOCAI *mAIObject;
	bool mEntered;
	AIState() : mEntered(false) { };

public:
	virtual ~AIState() {};

	void SetAI(GOCAI *ai) { mAIObject = ai; };

	virtual float GetPriority() const { return 0.0f; };

	void Enter()
	{
		if (!mEntered)
		{
			mEntered = true;
			OnEnter();
		}
		else OnContinue();
	}

	virtual void OnEnter() {};
	virtual void OnContinue() {};

	/*
	@return true wenn finished
	*/
	virtual bool Update(float time) = 0;

	virtual void Pause() {};
	virtual void Leave() {};

	bool operator < (const AIState &rhs) const
	{
		return (GetPriority() < (rhs.GetPriority()));
	}
	bool operator > (const AIState &rhs) const
	{
		return (GetPriority() > (rhs.GetPriority()));
	}
};

};