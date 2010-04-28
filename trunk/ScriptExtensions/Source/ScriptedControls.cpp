#include "ScriptedControls.h"
#include "IceMessageSystem.h"
#include "IceUtils.h"
#include "IceMain.h"

ScriptedControls&
ScriptedControls::GetInstance()
{
	static ScriptedControls r;
	return r;
}

ScriptedControls::ScriptedControls()
{
	Ice::MessageSystem::Instance().JoinNewsgroup(this, "CONTROL_DOWN");
	Ice::MessageSystem::Instance().JoinNewsgroup(this, "CONTROL_UP");

	Ice::ScriptSystem::GetInstance().ShareCFunction("input_set_control", SetControl);
	Ice::ScriptSystem::GetInstance().ShareCFunction("input_get_control", GetControl);
	Ice::ScriptSystem::GetInstance().ShareCFunction("input_set_control_down_callback", SetControlDownCallback);
	Ice::ScriptSystem::GetInstance().ShareCFunction("input_set_control_up_callback", SetControlUpCallback);
}

void
ScriptedControls::ReceiveMessage(Ice::Msg &msg)
{
	if(msg.type == "CONTROL_DOWN")
	{
		if(m_mControlDownCallbacks.find(msg.params.GetOgreString("CONTROL_NAME"))!=m_mControlDownCallbacks.end())
		{
			Ice::ScriptSystem::RunCallbackFunction(m_mControlDownCallbacks[msg.params.GetOgreString("CONTROL_NAME")], std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(msg.params.GetOgreString("CONTROL_NAME"))));
		}
	}
	if(msg.type == "CONTROL_UP")
	{
		if(m_mControlUpCallbacks.find(msg.params.GetOgreString("CONTROL_NAME"))!=m_mControlUpCallbacks.end())
		{
			Ice::ScriptSystem::RunCallbackFunction(m_mControlUpCallbacks[msg.params.GetOgreString("CONTROL_NAME")], std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(msg.params.GetOgreString("CONTROL_NAME"))));
		}
	}
}

std::vector<Ice::ScriptParam>
ScriptedControls::SetControl(Ice::Script &caller, std::vector<Ice::ScriptParam> params)
{
	std::vector<Ice::ScriptParam> errout(1, Ice::ScriptParam());
	std::vector<Ice::ScriptParam> vRef(1, Ice::ScriptParam(std::string("")));
	vRef.push_back(Ice::ScriptParam(0.1));
	std::string strErrString=Ice::Utils::TestParameters(params, vRef, false);
	if(strErrString.length())
		{errout.push_back(strErrString);return errout;}
	Ice::Main::Instance().GetInputManager()->SetControl(params[0].getString(), std::vector<std::pair<OIS::KeyCode, OIS::MouseButtonID>>(1, std::pair<OIS::KeyCode, OIS::MouseButtonID>((OIS::KeyCode)params[1].getInt(), OIS::MB_Left)));
	return std::vector<Ice::ScriptParam>();
}

std::vector<Ice::ScriptParam>
ScriptedControls::GetControl(Ice::Script &caller, std::vector<Ice::ScriptParam> params)
{
	std::vector<Ice::ScriptParam> errout(1, Ice::ScriptParam());
	errout.push_back(Ice::ScriptParam(std::string("not implemented!!")));
	return errout;
}

std::vector<Ice::ScriptParam>
ScriptedControls::SetControlDownCallback(Ice::Script &caller, std::vector<Ice::ScriptParam> params)
{
	std::vector<Ice::ScriptParam> errout(1, Ice::ScriptParam());
	std::vector<Ice::ScriptParam> vRef(1, Ice::ScriptParam(std::string("")));
	vRef.push_back(Ice::ScriptParam("", caller));
	std::string strErrString=Ice::Utils::TestParameters(params, vRef, false);
	if(strErrString.length()) {errout.push_back(strErrString);return errout;}
	GetInstance().m_mControlDownCallbacks[params[0].getString()]=params[1];
	return std::vector<Ice::ScriptParam>();
}

std::vector<Ice::ScriptParam>
ScriptedControls::SetControlUpCallback(Ice::Script &caller, std::vector<Ice::ScriptParam> params)
{
	std::vector<Ice::ScriptParam> errout(1, Ice::ScriptParam());
	std::vector<Ice::ScriptParam> vRef(1, Ice::ScriptParam(std::string("")));
	vRef.push_back(Ice::ScriptParam("", caller));
	std::string strErrString=Ice::Utils::TestParameters(params, vRef, false);
	if(strErrString.length())
		{errout.push_back(strErrString);return errout;}
	GetInstance().m_mControlUpCallbacks[params[0].getString()]=params[1];
	return std::vector<Ice::ScriptParam>();
}
