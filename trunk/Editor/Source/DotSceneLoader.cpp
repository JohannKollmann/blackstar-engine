
#include "DotSceneLoader.h"
#include "IceGameObject.h"
#include "IceGOCView.h"
#include "IceGOCPhysics.h"

DotSceneLoader::DotSceneLoader(void)
{
}

DotSceneLoader::~DotSceneLoader(void)
{
}

Ogre::String DotSceneLoader::GetStringAttribute(TiXmlElement* xmlElement, const char* name)
{
    const char* value = xmlElement->Attribute(name);
    if (value != 0)
        return value;
    else
		return Ogre::StringUtil::BLANK;    
}

float DotSceneLoader::GetRealAttribute(TiXmlElement* xmlElement, const char* name, float defaultValue)
{
	Ogre::String value = GetStringAttribute(xmlElement, name);
	return value.empty() ? defaultValue : Ogre::StringConverter::parseReal(value);    
}

void DotSceneLoader::LoadXYZ(TiXmlElement* objectElement, Ogre::Vector3& xyz)
{
    xyz.x = GetRealAttribute(objectElement, "x", 0);
    xyz.y = GetRealAttribute(objectElement, "y", 0);
    xyz.z = GetRealAttribute(objectElement, "z", 0);
}

Ogre::Quaternion DotSceneLoader::LoadRotation(TiXmlElement* objectElement)
{
	Ogre::Quaternion rotation = Ogre::Quaternion::IDENTITY;

    if (objectElement->Attribute("qx") != 0)
    {
        //The rotation is specified as a quaternion
        rotation.x = GetRealAttribute(objectElement, "qx", 0);
        rotation.y = GetRealAttribute(objectElement, "qy", 0);
        rotation.z = GetRealAttribute(objectElement, "qz", 0);
        rotation.w = GetRealAttribute(objectElement, "qw", 0);
    }
    else if (objectElement->Attribute("axisX") != 0)
    {
        //The rotation is specified as an axis and angle
        Ogre::Real angle = GetRealAttribute(objectElement, "angle", 0);

		Ogre::Vector3 axis;
        axis.x = GetRealAttribute(objectElement, "axisX", 0);
        axis.y = GetRealAttribute(objectElement, "axisY", 0);
        axis.z = GetRealAttribute(objectElement, "axisZ", 0);

        //Convert the angle and axis into the rotation quaternion
		rotation.FromAngleAxis(Ogre::Radian(angle), axis);
    }
    else if (objectElement->Attribute("angleX") != 0)
    {
        //Assume the rotation is specified as three Euler angles
		Ogre::Vector3 euler;
        euler.x = GetRealAttribute(objectElement, "angleX", 0);
        euler.y = GetRealAttribute(objectElement, "angleY", 0);
        euler.z = GetRealAttribute(objectElement, "angleZ", 0);
        Ogre::String order = GetStringAttribute(objectElement, "order");

        //Convert Euler angles to a matrix
		Ogre::Matrix3 rotationMatrix;
        if (order.length() < 2)
            rotationMatrix.FromEulerAnglesXYZ(Ogre::Radian(euler.x), Ogre::Radian(euler.y), Ogre::Radian(euler.z));
        else
        {
            if (order[0] == 'x')
            {
                if (order[1] == 'y')
                    rotationMatrix.FromEulerAnglesXYZ(Ogre::Radian(euler.x), Ogre::Radian(euler.y), Ogre::Radian(euler.z));
                else
                    rotationMatrix.FromEulerAnglesXZY(Ogre::Radian(euler.x), Ogre::Radian(euler.y), Ogre::Radian(euler.z));
            }
            else if (order[0] == 'y')
            {
                if (order[1] == 'x')
                    rotationMatrix.FromEulerAnglesYXZ(Ogre::Radian(euler.x), Ogre::Radian(euler.y), Ogre::Radian(euler.z));
                else
                    rotationMatrix.FromEulerAnglesYZX(Ogre::Radian(euler.x), Ogre::Radian(euler.y), Ogre::Radian(euler.z));
            }
            else
            {
                if (order[1] == 'x')
                    rotationMatrix.FromEulerAnglesZXY(Ogre::Radian(euler.x), Ogre::Radian(euler.y), Ogre::Radian(euler.z));
                else
                    rotationMatrix.FromEulerAnglesZYX(Ogre::Radian(euler.x), Ogre::Radian(euler.y), Ogre::Radian(euler.z));
            }
        }

        //Convert the matrix into the rotation quaternion
        rotation.FromRotationMatrix(rotationMatrix);
    }
    
    return rotation;
}

void DotSceneLoader::LoadNode(TiXmlElement* objectElement)
{
	Ogre::String elementName;
	TiXmlElement* childElement = objectElement->FirstChildElement();
	Ogre::Vector3 position;
	Ogre::Vector3 scale = Ogre::Vector3(1,1,1);
	Ogre::Quaternion rotation;
	Ogre::String meshname;
    while(childElement)
    {
        elementName = childElement->Value();

        if (elementName == "position")
            LoadXYZ(childElement, position);
        else if (elementName == "rotation")
            rotation = LoadRotation(childElement);
        else if (elementName == "scale")
            LoadXYZ(childElement, scale);
        else if (elementName == "entity")
			meshname = GetStringAttribute(childElement, "meshFile");   
		childElement = childElement->NextSiblingElement();
    }
	if (meshname != "")
	{
		Ice::GameObject *object = new Ice::GameObject();
		Ice::GOCViewContainer *visuals = new Ice::GOCViewContainer();
		visuals->AddItem(new Ice::MeshRenderable(meshname, true));
		object->AddComponent(new Ice::GOCStaticBody(meshname));
		object->AddComponent(visuals);
		object->SetGlobalPosition(position);
		object->SetGlobalScale(scale);
		object->SetGlobalOrientation(rotation);
		object->SetSelectable(false);
	}
}

void DotSceneLoader::LoadNodes(TiXmlElement* element)
{
	TiXmlElement* childElement = element->FirstChildElement();
    while(childElement)
    {
		Ogre::String elementName = childElement->Value();
        if (elementName == "node" || elementName == "modelInstance")
            LoadNode(childElement);

		childElement = childElement->NextSiblingElement();
    }
}

void DotSceneLoader::ImportScene(Ogre::String fileName)
{
	TiXmlDocument document;
	Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton().openResource(fileName, "General");
    if (stream.isNull())
    {
		Ogre::StringUtil::StrStreamType errorMessage;
        errorMessage << "Could not open scene file: " << fileName;

		OGRE_EXCEPT
            (
            Ogre::Exception::ERR_FILE_NOT_FOUND,
		    errorMessage.str(), 
		    "OgreMaxUtilities::LoadXmlDocument"
            );
    }

    //Get the file contents
	Ogre::String data = stream->getAsString();
    
    //Parse the XML document
    document.Parse(data.c_str());
    stream.setNull();
    if (document.Error())
    {
        Ogre::StringUtil::StrStreamType errorMessage;
        errorMessage << "There was an error with the scene file: " << fileName;

        OGRE_EXCEPT
            (
            Ogre::Exception::ERR_INVALID_STATE,
		    errorMessage.str(), 
		    "OgreMaxUtilities::LoadXmlDocument"
            );
    }

	TiXmlElement* rootElement = document.FirstChildElement();
	TiXmlElement* childElement = rootElement->FirstChildElement();
    while(childElement)
    {
		Ogre::String elementName = childElement->Value();
        
        if (elementName == "nodes")
            LoadNodes(childElement);  
		childElement = childElement->NextSiblingElement();
	}
}

DotSceneLoader& DotSceneLoader::Instance()
{
	static DotSceneLoader TheOneAndOnly;
	return TheOneAndOnly;
};