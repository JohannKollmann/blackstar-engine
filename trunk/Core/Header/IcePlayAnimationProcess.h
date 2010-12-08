#pragma once

#include "IceIncludes.h"
#include "IceScriptSystem.h"
#include "Ogre.h"
#include "IceProcessNode.h"
#include "IceMessageSystem.h"

namespace Ice
{

	class DllExport PlayAnimationProcess : public ProcessNode, public MessageListener
	{

	private:
		Ogre::AnimationState *mAnimationState;
		bool mLooped;
		float mInBlendDuration;
		float mOutBlendDuration;
		float mTimeScale;
		bool mBlendingIn;
		bool mBlendingOut;
		float mWeight;

		struct Callback { float timePos; ScriptParam callback; };
		std::vector<Callback> mScriptCallbacks;

	protected:
		void OnSetActive(bool active);

	public:
		PlayAnimationProcess(Ogre::AnimationState *state);
		~PlayAnimationProcess();
		void ReceiveMessage(Msg &msg) override;

		void SetLooped(bool looped) { mLooped = looped; }
		void SetInBlendDuration(float duration) { mInBlendDuration = duration; }
		void SetOutBlendDuration(float duration) { mOutBlendDuration = duration; }
		void SetTimeScale(float scale) { mTimeScale = scale; }

		void AddCallback(float timePos, ScriptParam callback);
	};
}