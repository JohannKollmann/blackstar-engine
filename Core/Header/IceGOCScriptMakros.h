
#pragma once

#include "IceIncludes.h"
#include "IceGOComponent.h"
#include "IceGOCEditorInterface.h"
#include "IceUtils.h"
#include "IceSceneManager.h"
#include "IceGameObject.h"

namespace Ice
{
	#define DEFINE_GOCLUAMETHOD(gocClassName, methodName) \
	static std::vector<ScriptParam> Lua_##methodName (Script& caller, std::vector<ScriptParam> vParams) \
	{ \
		std::vector<ScriptParam> ref; \
		float fdummy = 0; \
		ref.push_back(ScriptParam(fdummy)); \
		Ogre::String param_test = Ice::Utils::TestParameters(vParams, ref, true); \
		if (param_test == "") \
		{ \
			int id = vParams[0].getInt(); \
			GameObject *obj = SceneManager::Instance().GetObjectByInternID(id); \
			if (!obj) \
			{ \
				return std::vector<ScriptParam>(); \
			} \
			gocClassName *component = obj->GetComponent<##gocClassName##>(); \
			if (!component) \
			{ \
				return std::vector<ScriptParam>(); \
			} \
			std::vector<ScriptParam> vObjParams; \
			for (unsigned int i = 0; i < vParams.size(); i++) vObjParams.push_back(vParams[i]);	/*pop the script id*/	\
			return component->##methodName##(caller, vObjParams); \
		} \
		else Ice::Utils::LogParameterErrors(caller, param_test); \
		return std::vector<ScriptParam>(); \
	}

	#define DEFINE_TYPEDGOCLUAMETHOD(gocClassName, methodName, sRefParams) \
	static std::vector<ScriptParam> Lua_##methodName (Script& caller, std::vector<ScriptParam> vParams) \
	{ \
		std::vector<ScriptParam> ref; \
		float fdummy = 0; \
		ref.push_back(ScriptParam(fdummy)); \
		std::string param_test = Ice::Utils::TestParameters(vParams, ref, true); \
		if (param_test == "") \
		{ \
			int id = vParams[0].getInt(); \
			GameObject *obj = SceneManager::Instance().GetObjectByInternID(id); \
			if (!obj) \
			{ \
				return std::vector<ScriptParam>(); \
			} \
			gocClassName *component = obj->GetComponent<##gocClassName##>(); \
			if (!component) \
			{ \
				return std::vector<ScriptParam>(); \
			} \
			std::vector<ScriptParam> vObjParams; \
			for (unsigned int i = 0; i < vParams.size(); i++) vObjParams.push_back(vParams[i]);	/*pop the script id*/	\
			Ogre::String param_test2 = Ice::Utils::TestParameters(vParams, sRefParams, true); \
			if (param_test == "") return component->##methodName##(caller, vObjParams); \
			else Ice::Utils::LogParameterErrors(caller, param_test2); \
		} \
		else Ice::Utils::LogParameterErrors(caller, param_test); \
		return std::vector<ScriptParam>(); \
	}

	#define DEFINE_GOLUAMETHOD_CPP(methodName) \
	std::vector<ScriptParam> GameObject::Lua_##methodName (Script& caller, std::vector<ScriptParam> vParams) \
	{ \
		std::vector<ScriptParam> ref; \
		float fdummy = 0; \
		ref.push_back(ScriptParam(fdummy)); \
		std::string param_test = Ice::Utils::TestParameters(vParams, ref, true); \
		if (param_test == "") \
		{ \
			int id = vParams[0].getInt(); \
			GameObject *obj = SceneManager::Instance().GetObjectByInternID(id); \
			if (!obj) \
			{ \
				return std::vector<ScriptParam>(); \
			} \
			std::vector<ScriptParam> vObjParams; \
			for (unsigned int i = 0; i < vParams.size(); i++) vObjParams.push_back(vParams[i]);	/*pop the script id*/	\
			return obj->##methodName##(caller, vObjParams); \
		} \
		else Ice::Utils::LogParameterErrors(caller, param_test); \
		return std::vector<ScriptParam>(); \
	}

	#define DEFINE_TYPEDGOLUAMETHOD_CPP(methodName, sRefParams) \
	std::vector<ScriptParam> GameObject::Lua_##methodName (Script& caller, std::vector<ScriptParam> vParams) \
	{ \
		std::vector<ScriptParam> ref; \
		float fdummy = 0; \
		ref.push_back(ScriptParam(fdummy)); \
		std::string param_test = Ice::Utils::TestParameters(vParams, ref, true); \
		if (param_test == "") \
		{ \
			int id = vParams[0].getInt(); \
			GameObject *obj = SceneManager::Instance().GetObjectByInternID(id); \
			if (!obj) \
			{ \
				return std::vector<ScriptParam>(); \
			} \
			std::vector<ScriptParam> vObjParams; \
			for (unsigned int i = 0; i < vParams.size(); i++) vObjParams.push_back(vParams[i]);	/*pop the script id*/	\
			Ogre::String param_test2 = Ice::Utils::TestParameters(vParams, sRefParams, true); \
			if (param_test == "") return obj->##methodName##(caller, vObjParams); \
			else Ice::Utils::LogParameterErrors(caller, param_test2); \
		} \
		else Ice::Utils::LogParameterErrors(caller, param_test); \
		return std::vector<ScriptParam>(); \
	}

}
	