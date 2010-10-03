#pragma once

#include "Ogre.h"

namespace Ice
{

	class HDRListener : public Ogre::CompositorInstance::Listener
	{
		class LumListener : public Ogre::RenderTargetListener
		{
		public:
			float mUpdateTimeInterval;
			float mLastTimeUpdated;
			float avgLuminence;
			void postRenderTargetUpdate(const Ogre::RenderTargetEvent &evt);
		};

	protected:
		int mVpWidth, mVpHeight;
		int mBloomSize;
		Ogre::CompositorInstance *mCompositor;
		LumListener mLumListener;

		// Array params - have to pack in groups of 4 since this is how Cg generates them
		// also prevents dependent texture read problems if ops don't require swizzle
		float mBloomTexOffsetsHorz[25][4];
		float mBloomTexOffsetsVert[25][4];

		float mBloomGlareTexOffset[25][4];

	public:
		HDRListener();
		virtual ~HDRListener();
		void notifyViewportSize(int width, int height);
		void notifyCompositor(Ogre::CompositorInstance* instance);
		virtual void notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat);
		void notifyResourcesCreated(bool forResizeOnly);
		virtual void notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat);
	};

	class VolumetricLightListener : public Ogre::CompositorInstance::Listener
	{
		void notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat);
	};
};