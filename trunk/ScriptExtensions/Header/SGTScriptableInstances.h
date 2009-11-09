#include "SGTScriptSystem.h"
#include "SGTIncludes.h"
#include "SGTScriptSystem.h"
#include <map>

class SGTScriptableInstances
{
public:
	SGTScriptableInstances();
	static SGTScriptableInstances& GetInstance();
	void Clear();
private:
	std::map<int, SGTScript> m_mScripts;

	static std::vector<SGTScriptParam> Lua_InstantiateScript(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_RunFunction(SGTScript& caller, std::vector<SGTScriptParam> vParams);
};