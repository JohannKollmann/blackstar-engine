#include <iostream>
#include "SGTScriptSystem.h"

std::vector<SGTScriptParam>
DummyShare(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	for(unsigned int iArg=0; iArg<vParams.size(); iArg++)
		if(vParams[iArg].getType()==SGTScriptParam::PARM_TYPE_STRING)
			std::cout<<vParams[iArg].getString();
	return std::vector<SGTScriptParam>();
}

void main()
{
	std::cout<<"tester\n------\n\n";
	SGTScriptSystem::GetInstance().ShareCFunction("CShare", DummyShare);
	SGTScriptSystem::GetInstance().CreateInstance(std::string("a.lua"));
}