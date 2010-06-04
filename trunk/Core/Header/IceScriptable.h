
#pragma once

#include "IceIncludes.h"
#include "OgreString.h"
#include "IceScriptSystem.h"
#include "IceMessageListener.h"

namespace Ice
{

	#define DEFINE_OBJECTSCRIPTFUNCTION(methodID) \
	static std::vector<ScriptParam> Lua_ ## methodID (Script& caller, std::vector<ScriptParam> vParams) \
	{ \
		std::vector<ScriptParam> ref; \
		float fdummy = 0; \
		Scriptable *obj = nullptr; \
		ref.push_back(ScriptParam(fdummy)); \
		Ogre::String param_test = Ice::Utils::TestParameters(vParams, ref, true); \
		if (param_test == "") \
		{ \
			int id = vParams[0].getInt(); \
			obj = Ice::ScriptSystem::Instance().GetScriptableObject(id); \
			std::vector<ScriptParam> vObjParams; \
			for (unsigned int i = 0; i < vParams.size(); i++) vObjParams.push_back(vParams[i]);	/*pop the script id*/	\
			return obj->ScriptCall(methodID, vObjParams); \
		} \
		else Ice::Utils::LogParameterErrors(caller, param_test); \
		return std::vector<ScriptParam>(); \
	}
	#define DECLARE_OBJECTSCRIPTFUNCTION(methodID) ScriptSystem::GetInstance().ShareCFunction(#methodID, &Lua_methodID);

	class DllExport Scriptable : MessageListener
	{
	protected:
		Script mScript;
		Ogre::String mScriptFileName;
		std::map<std::string, ScriptParam> mProperties;

		void InitScript(Ogre::String scriptFilename);

		virtual void OnScriptReload() {}

		typedef Ogre::String MethodID;

	public:

		virtual ~Scriptable();

		/*
		Handles Script reparse functionality and calls OnReceiveMessage
		*/
		void ReceiveMessage(const Msg& msg);

		virtual void OnReceiveMessage(const Msg& msg) {}

		/*
		ScriptCall handles property get/set methods and calls OnScriptCall
		*/
		virtual std::vector<ScriptParam> ScriptCall(MethodID method, std::vector<ScriptParam> &params);

		virtual std::vector<ScriptParam> OnScriptCall(MethodID method, std::vector<ScriptParam> &params) = 0;
	};
}