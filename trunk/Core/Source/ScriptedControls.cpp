#include "ScriptedControls.h"
#include "SGTMessageSystem.h"
#include "SGTUtils.h"
#include "SGTMain.h"

ScriptedControls&
ScriptedControls::GetInstance()
{
	static ScriptedControls r;
	return r;
}

ScriptedControls::ScriptedControls()
{
	SGTMessageSystem::Instance().JoinNewsgroup(this, "CONTROL_DOWN");
	SGTMessageSystem::Instance().JoinNewsgroup(this, "CONTROL_UP");

	SGTScriptSystem::GetInstance().ShareCFunction("input_set_control", SetControl);
	SGTScriptSystem::GetInstance().ShareCFunction("input_get_control", GetControl);
	SGTScriptSystem::GetInstance().ShareCFunction("input_set_control_down_callback", SetControlDownCallback);
	SGTScriptSystem::GetInstance().ShareCFunction("input_set_control_up_callback", SetControlUpCallback);
}

void
ScriptedControls::ReceiveMessage(SGTMsg &msg)
{
	if(msg.mNewsgroup == "CONTROL_DOWN")
	{
		if(m_mControlDownCallbacks.find(msg.mData.GetOgreString("CONTROL_NAME"))!=m_mControlDownCallbacks.end())
		{
			SGTScriptSystem::RunCallbackFunction(m_mControlDownCallbacks[msg.mData.GetOgreString("CONTROL_NAME")], std::vector<SGTScriptParam>(1, SGTScriptParam(msg.mData.GetOgreString("CONTROL_NAME"))));
		}
	}
	if(msg.mNewsgroup == "CONTROL_UP")
	{
		if(m_mControlUpCallbacks.find(msg.mData.GetOgreString("CONTROL_NAME"))!=m_mControlUpCallbacks.end())
		{
			SGTScriptSystem::RunCallbackFunction(m_mControlUpCallbacks[msg.mData.GetOgreString("CONTROL_NAME")], std::vector<SGTScriptParam>(1, SGTScriptParam(msg.mData.GetOgreString("CONTROL_NAME"))));
		}
	}
}

std::vector<SGTScriptParam>
ScriptedControls::SetControl(SGTScript &caller, std::vector<SGTScriptParam> params)
{
	std::vector<SGTScriptParam> errout(1, SGTScriptParam());
	std::vector<SGTScriptParam> vRef(1, SGTScriptParam(std::string("")));
	vRef.push_back(SGTScriptParam(0.1));
	std::string strErrString=SGTUtils::TestParameters(params, vRef, false);
	if(strErrString.length())
		{errout.push_back(strErrString);return errout;}
	SGTMain::Instance().GetInputManager()->SetControl(params[0].getString(), std::vector<std::pair<OIS::KeyCode, OIS::MouseButtonID>>(1, std::pair<OIS::KeyCode, OIS::MouseButtonID>((OIS::KeyCode)params[1].getInt(), OIS::MB_Left)));
	return std::vector<SGTScriptParam>();
}

std::vector<SGTScriptParam>
ScriptedControls::GetControl(SGTScript &caller, std::vector<SGTScriptParam> params)
{
	std::vector<SGTScriptParam> errout(1, SGTScriptParam());
	errout.push_back(SGTScriptParam(std::string("not implemented!!")));
	return errout;
}

std::vector<SGTScriptParam>
ScriptedControls::SetControlDownCallback(SGTScript &caller, std::vector<SGTScriptParam> params)
{
	std::vector<SGTScriptParam> errout(1, SGTScriptParam());
	std::vector<SGTScriptParam> vRef(1, SGTScriptParam(std::string("")));
	vRef.push_back(SGTScriptParam("", caller));
	std::string strErrString=SGTUtils::TestParameters(params, vRef, false);
	if(strErrString.length()) {errout.push_back(strErrString);return errout;}
	GetInstance().m_mControlDownCallbacks[params[0].getString()]=params[1];
	return std::vector<SGTScriptParam>();
}

std::vector<SGTScriptParam>
ScriptedControls::SetControlUpCallback(SGTScript &caller, std::vector<SGTScriptParam> params)
{
	std::vector<SGTScriptParam> errout(1, SGTScriptParam());
	std::vector<SGTScriptParam> vRef(1, SGTScriptParam(std::string("")));
	vRef.push_back(SGTScriptParam("", caller));
	std::string strErrString=SGTUtils::TestParameters(params, vRef, false);
	if(strErrString.length())
		{errout.push_back(strErrString);return errout;}
	GetInstance().m_mControlUpCallbacks[params[0].getString()]=params[1];
	return std::vector<SGTScriptParam>();
}
