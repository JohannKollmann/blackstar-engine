#include "IceScriptSystem.h"
#include "IceIncludes.h"
#include "IceScriptSystem.h"
#include <map>

class ScriptableInstances
{
public:
	ScriptableInstances();
	static ScriptableInstances& GetInstance();
	void Clear();
private:
	std::map<int, Ice::Script> m_mScripts;

	static std::vector<Ice::ScriptParam> Lua_InstantiateScript(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_RunFunction(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
};