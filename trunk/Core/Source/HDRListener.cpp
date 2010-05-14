#include "HDRListener.h"

namespace Ice
{

	HDRListener::HDRListener()
	{
		mLumListener.avgLuminence = 1;
	}
	//---------------------------------------------------------------------------
	HDRListener::~HDRListener()
	{
	}
	//---------------------------------------------------------------------------
	void HDRListener::notifyViewportSize(int width, int height)
	{
		mVpWidth = width;
		mVpHeight = height;
	}

	void HDRListener::notifyResourcesCreated(bool forResizeOnly)
	{
		mCompositor->getRenderTarget("rt_lum0")->removeAllListeners();
		mCompositor->getRenderTarget("rt_lum0")->addListener(&mLumListener);
	}
	//---------------------------------------------------------------------------
	void HDRListener::notifyCompositor(Ogre::CompositorInstance* instance)
	{
		mCompositor = instance;

		// Get some RTT dimensions for later calculations
		Ogre::CompositionTechnique::TextureDefinitionIterator defIter =
			instance->getTechnique()->getTextureDefinitionIterator();
		
		while (defIter.hasMoreElements())
		{
			Ogre::CompositionTechnique::TextureDefinition* def =
				defIter.getNext();

			if(def->name == "rt_bloomHorz0")
			{
				mBloomSize = (int)def->width; // should be square
				// Calculate gaussian texture offsets & weights
				float deviation = 5.0f;


				//Glare bloom 1
				int index = 0;
				float glareScale = 1.0f;
				for (int i = 0; i < 25; i++)
				{
					//for (int n = 0; n < 4; n++) mBloomGlareWeights[i][n] = 0.0f;
					mBloomGlareTexOffset[i][0] = 0;
					mBloomGlareTexOffset[i][1] = 0;
					mBloomGlareTexOffset[i][2] = 0;
				}
				float allWeights = 0;
				for (int x = -2; x <= 2; x++)
				{
					for (int y = -2; y <= 2; y++)
					{
						mBloomGlareTexOffset[index][0] = static_cast<float>(x);
						mBloomGlareTexOffset[index][1] = static_cast<float>(y);
						float gausx = Ogre::Math::Sqrt(static_cast<float>((x*x) + (y*y)));
						float w = Ogre::Math::gaussianDistribution(gausx, 0, 2); 
						mBloomGlareTexOffset[index][2] = w;
						allWeights += w;
						index++;
					}
				}
				for (int i = 0; i < 25; i++)
					mBloomGlareTexOffset[i][2] /= allWeights;



				allWeights = 0;
				// central sample, no offset
				mBloomTexOffsetsHorz[0][0] = 0.0f;
				mBloomTexOffsetsHorz[0][1] = 0.0f;
				mBloomTexOffsetsVert[0][0] = 0.0f;
				mBloomTexOffsetsVert[0][1] = 0.0f;

				mBloomTexOffsetsHorz[0][2] = Ogre::Math::gaussianDistribution(0, 0, deviation);
				mBloomTexOffsetsVert[0][2] = mBloomTexOffsetsHorz[0][2];
				allWeights+=mBloomTexOffsetsHorz[0][2];

				// 'pre' samples
				for(int i = 1; i < 13; i++)
				{
					mBloomTexOffsetsHorz[i][0] = static_cast<float>(i);
					mBloomTexOffsetsHorz[i][1] = 0.0f;
					mBloomTexOffsetsVert[i][0] = 0.0f;
					mBloomTexOffsetsVert[i][1] = static_cast<float>(i);

					mBloomTexOffsetsHorz[i][2] = Ogre::Math::gaussianDistribution(static_cast<float>(i), 0, deviation);
					mBloomTexOffsetsVert[i][2] = mBloomTexOffsetsHorz[i][2];
					allWeights += mBloomTexOffsetsHorz[i][2];
				}
				// 'post' samples
				for(int i = 13; i < 25; i++)
				{
					mBloomTexOffsetsHorz[i][0] = -mBloomTexOffsetsHorz[i - 12][0];
					mBloomTexOffsetsHorz[i][1] = 0.0f;
					mBloomTexOffsetsVert[i][0] = 0.0f;
					mBloomTexOffsetsVert[i][1] = -mBloomTexOffsetsVert[i - 12][1];

					mBloomTexOffsetsHorz[i][2] = mBloomTexOffsetsHorz[i - 12][2];
					mBloomTexOffsetsVert[i][2] = mBloomTexOffsetsHorz[i][2];
					allWeights += mBloomTexOffsetsHorz[i][2];
				}
				for (int i = 0; i < 25; i++)
				{
					mBloomTexOffsetsHorz[i][2] /= allWeights;
					mBloomTexOffsetsVert[i][2] /= allWeights;
				}

			}
		}
	}
	//---------------------------------------------------------------------------
	void HDRListener::notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
	{
		// Prepare the fragment params offsets
		switch(pass_id)
		{
		case 701:
			{
				// horizontal bloom
				mat->load();
				Ogre::GpuProgramParametersSharedPtr fparams =
					mat->getBestTechnique()->getPass(0)->getFragmentProgramParameters();
				const Ogre::String& progName = mat->getBestTechnique()->getPass(0)->getFragmentProgramName();
				fparams->setNamedConstant("samples", mBloomTexOffsetsHorz[0], 25);

				break;
			}
			break;
		case 702:
			{
				// vertical bloom
				mat->load();
				Ogre::GpuProgramParametersSharedPtr fparams =
					mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
				const Ogre::String& progName = mat->getBestTechnique()->getPass(0)->getFragmentProgramName();
				fparams->setNamedConstant("samples", mBloomTexOffsetsVert[0], 25);

				break;
			}
		case 703:
			{
				// glare bloom
				mat->load();
				Ogre::GpuProgramParametersSharedPtr fparams =
					mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
				const Ogre::String& progName = mat->getBestTechnique()->getPass(0)->getFragmentProgramName();
				fparams->setNamedConstant("samples", mBloomGlareTexOffset[0], 25);

				break;
			}
		}
	}
	//---------------------------------------------------------------------------
	void HDRListener::notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
	{
		if (pass_id == 990)
		{
			mat->load();
			Ogre::GpuProgramParametersSharedPtr fparams =
				mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
			fparams->setNamedConstant("oldAvgLum", mLumListener.avgLuminence);
		}
	}


	void HDRListener::LumListener::postRenderTargetUpdate( const Ogre::RenderTargetEvent &evt )
	{
		Ogre::PixelBox pbx(1,1,1,Ogre::PixelFormat::PF_FLOAT32_R, &avgLuminence);
		evt.source->copyContentsToMemory(pbx);
		//Ogre::LogManager::getSingleton().logMessage(Ogre::StringConverter::toString(avgLuminence));
	}

};