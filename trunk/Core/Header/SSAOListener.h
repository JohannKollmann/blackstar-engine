
#pragma once

#include "Ogre.h"
#include "SGTIncludes.h"
#include "SGTMain.h"

class SGTDllExport SSAOListener: public Ogre::CompositorInstance::Listener
{
public:
	SSAOListener() {}
	~SSAOListener() {}
    // this callback we will use to modify SSAO parameters
    void notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
    {
        if (pass_id != 42) // not SSAO, return
            return;

        // this is the camera you're using
		Ogre::Camera *cam = SGTMain::Instance().GetCamera();

        // calculate the far-top-right corner in view-space
        Ogre::Vector3 farCorner = cam->getViewMatrix(true) * cam->getWorldSpaceCorners()[4];

        // get the pass
        Ogre::Pass *pass = mat->getBestTechnique()->getPass(0);

        // get the vertex shader parameters
        Ogre::GpuProgramParametersSharedPtr params = pass->getVertexProgramParameters();
        // set the camera's far-top-right corner
        if (params->_findNamedConstantDefinition("farCorner"))
            params->setNamedConstant("farCorner", farCorner);

        // get the fragment shader parameters
        params = pass->getFragmentProgramParameters();
        // set the projection matrix we need
        static const Ogre::Matrix4 CLIP_SPACE_TO_IMAGE_SPACE(
            0.5,    0,    0,  0.5,
            0,   -0.5,    0,  0.5,
            0,      0,    1,    0,
            0,      0,    0,    1);
        if (params->_findNamedConstantDefinition("ptMat"))
            params->setNamedConstant("ptMat", CLIP_SPACE_TO_IMAGE_SPACE * cam->getProjectionMatrixWithRSDepth());
        if (params->_findNamedConstantDefinition("far"))
            params->setNamedConstant("far", cam->getFarClipDistance());
    }
};