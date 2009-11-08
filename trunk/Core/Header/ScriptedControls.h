#include "SGTScriptSystem.h"
#include "SGTInput.h"

class ScriptedControls : public SGTMessageListener
{
public:
	ScriptedControls();
	static ScriptedControls& GetInstance();

	void ReceiveMessage(SGTMsg &msg);
private:
	std::map<Ogre::String, SGTScriptParam> m_mControlDownCallbacks;
	std::map<Ogre::String, SGTScriptParam> m_mControlUpCallbacks;
	
	static std::vector<SGTScriptParam> SetControl(SGTScript &caller, std::vector<SGTScriptParam> params);
	static std::vector<SGTScriptParam> GetControl(SGTScript &caller, std::vector<SGTScriptParam> params);
	static std::vector<SGTScriptParam> SetControlDownCallback(SGTScript &caller, std::vector<SGTScriptParam> params);
	static std::vector<SGTScriptParam> SetControlUpCallback(SGTScript &caller, std::vector<SGTScriptParam> params);
};