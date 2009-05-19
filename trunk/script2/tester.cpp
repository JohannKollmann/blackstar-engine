#include <iostream>
#include <fstream>
#include "SGTScriptSystem.h"

#include "ResidentVariables.h"



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
	std::ifstream file("test.bin");
	ResidentVariables* pResis;
	ResidentManager::GetInstance();//init
	if(file.good())
	{
		SGTLoadSystem* pLS=SGTLoadSave::Instance().LoadFile("test.bin");
		pResis=(ResidentVariables*)pLS->LoadObject();
		pLS->CloseFile();
	}
	else
		pResis=new ResidentVariables;

	SGTScript script=SGTScriptSystem::GetInstance().CreateInstance(std::string("a.lua"));
	ResidentManager::GetInstance().BindResisToScript(*pResis, script);
	script.CallFunction("increase_resi", std::vector<SGTScriptParam>());

	SGTSaveSystem* pSS=SGTLoadSave::Instance().CreateSaveFile("test.bin", "..\\LoadSave_dll\\xsl\\resi_test.xml");
	pSS->SaveObject((SGTSaveable*)&ResidentManager::GetInstance().GetResis(script), "Resis");
	pSS->CloseFiles();

	delete pResis;
}