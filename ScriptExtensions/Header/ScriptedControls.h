#include "IceScriptSystem.h"
#include "IceInput.h"

class ScriptedControls : public Ice::MessageListener
{
public:
	ScriptedControls();
	static ScriptedControls& GetInstance();

	void ReceiveMessage(Ice::Msg &msg);
private:
	std::map<Ogre::String, Ice::ScriptParam> m_mControlDownCallbacks;
	std::map<Ogre::String, Ice::ScriptParam> m_mControlUpCallbacks;
	
	static std::vector<Ice::ScriptParam> SetControl(Ice::Script &caller, std::vector<Ice::ScriptParam> params);
	static std::vector<Ice::ScriptParam> GetControl(Ice::Script &caller, std::vector<Ice::ScriptParam> params);
	static std::vector<Ice::ScriptParam> SetControlDownCallback(Ice::Script &caller, std::vector<Ice::ScriptParam> params);
	static std::vector<Ice::ScriptParam> SetControlUpCallback(Ice::Script &caller, std::vector<Ice::ScriptParam> params);
};