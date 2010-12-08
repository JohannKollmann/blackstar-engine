
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
		std::vector<ScriptParam> errout;\
		errout.push_back(Ice::ScriptParam());\
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
				errout.push_back("could not find object for given ID!");\
				return errout;\
			} \
			gocClassName *component = obj->GetComponent<##gocClassName##>(); \
			if (!component) \
			{ \
				return std::vector<ScriptParam>(); \
			} \
			std::vector<ScriptParam> vObjParams; \
			for (unsigned int i = 1; i < vParams.size(); i++) vObjParams.push_back(vParams[i]);	/*pop the script id*/	\
			return component->##methodName##(caller, vObjParams); \
		} \
		else \
		{ \
			errout.push_back(param_test); \
			return errout; \
		} \
		return std::vector<ScriptParam>(); \
	}

	#define DEFINE_TYPEDGOCLUAMETHOD(gocClassName, methodName, sRefParams) \
	static std::vector<ScriptParam> Lua_##methodName (Script& caller, std::vector<ScriptParam> vParams) \
	{ \
		std::vector<ScriptParam> errout;\
		errout.push_back(Ice::ScriptParam());\
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
				errout.push_back("could not find object for given ID!");\
				return errout;\
			} \
			gocClassName *component = obj->GetComponent<##gocClassName##>(); \
			if (!component) \
			{ \
				errout.push_back("could not find component in given object!");\
				return errout;\
			} \
			std::vector<ScriptParam> vObjParams; \
			for (unsigned int i = 1; i < vParams.size(); i++) vObjParams.push_back(vParams[i]);	/*pop the script id*/	\
			Ogre::String param_test2 = Ice::Utils::TestParameters(vObjParams, sRefParams, true); \
			if (param_test2 == "") return component->##methodName##(caller, vObjParams); \
			else \
			{ \
				errout.push_back(param_test2); \
				return errout; \
			} \
		} \
		else \
		{ \
			errout.push_back(param_test); \
			return errout; \
		} \
		return std::vector<ScriptParam>(); \
	}

	#define DEFINE_GOLUAMETHOD_CPP(methodName) \
	std::vector<ScriptParam> GameObject::Lua_##methodName (Script& caller, std::vector<ScriptParam> vParams) \
	{ \
		std::vector<ScriptParam> errout;\
		errout.push_back(Ice::ScriptParam());\
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
				errout.push_back("could not find object for given ID!");\
				return errout;\
				/*std::vector<ScriptParam> errout; \
				bool b = false; \
				errout.push_back(ScriptParam(b)); \
				return errout; */ \
			} \
			std::vector<ScriptParam> vObjParams; \
			for (unsigned int i = 1; i < vParams.size(); i++) vObjParams.push_back(vParams[i]);	/*pop the script id*/	\
			return obj->##methodName##(caller, vObjParams); \
		} \
		else \
		{ \
			errout.push_back(param_test); \
			return errout; \
		} \
		return std::vector<ScriptParam>(); \
	}

	#define DEFINE_TYPEDGOLUAMETHOD_CPP(methodName, sRefParams) \
	std::vector<ScriptParam> GameObject::Lua_##methodName (Script& caller, std::vector<ScriptParam> vParams) \
	{ \
		std::vector<ScriptParam> errout;\
		errout.push_back(Ice::ScriptParam());\
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
				errout.push_back("could not find object for given ID!");\
				return errout;\
				/*std::vector<ScriptParam> errout; \
				bool b = false; \
				errout.push_back(ScriptParam(b)); \
				return errout; */ \
			} \
			std::vector<ScriptParam> vObjParams; \
			for (unsigned int i = 1; i < vParams.size(); i++) vObjParams.push_back(vParams[i]);	/*pop the script id*/	\
			Ogre::String param_test2 = Ice::Utils::TestParameters(vObjParams, sRefParams, true); \
			if (param_test2 == "") return obj->##methodName##(caller, vObjParams); \
			else \
			{ \
				errout.push_back(param_test2); \
				return errout; \
			} \
		} \
		else \
		{ \
			errout.push_back(param_test); \
			return errout; \
		} \
		return std::vector<ScriptParam>(); \
	}

}
	