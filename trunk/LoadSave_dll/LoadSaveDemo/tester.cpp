#include <iostream>
#include "..\LoadSave\ExternalHeaders\loadsave.h"
#include "standard_atoms.h"

void PrintText(std::string str)
{
	std::cout<<str;
}

class TestVector : LoadSave::Saveable
{
public:
	TestVector(){m_strName="TestVector";}
	void Save(LoadSave::SaveSystem& ss);
	void Load(LoadSave::LoadSystem& ls);
	static LoadSave::Saveable* NewInstance() {return new TestVector;}
	std::string& TellName(){return m_strName;}
	static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn){std::string str="TestVector"; *pstrName=str; *pFn=(LoadSave::SaveableInstanceFn)&NewInstance;}

	std::vector<float> vVector;
private:
	std::string m_strName;
};

void
TestVector::Load(LoadSave::LoadSystem &ls)
{
	ls.LoadAtom("std::vector<float>", &vVector);
}

void TestVector::Save(LoadSave::SaveSystem& ss)
{
	ss.SaveAtom("std::vector<float>", &vVector, "vVector");
}

class TestClass : LoadSave::Saveable
{
public:
	TestClass(){m_strName="TestClass";}
	void Init(){i1337=1337;i23=23;m_strHello="hello, world!";for(int i=0; i<50; i++){m_avVectors[i/5][i%5].vVector.push_back((float)i);m_avVectors[i/5][i%5].vVector.push_back((float)i+0.1f/0.3f);m_avVectors[i/5][i%5].vVector.push_back((float)i+0.2f/0.3f);}}
	void Save(LoadSave::SaveSystem& ss);
	void Load(LoadSave::LoadSystem& ls);
	static LoadSave::Saveable* NewInstance() {return new TestClass;}
	std::string& TellName(){return m_strName;}
	static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn){std::string str="TestClass"; *pstrName=str; *pFn=(LoadSave::SaveableInstanceFn)&NewInstance;}
private:
	std::string m_strName;
	int i1337;
	int i23;
	std::string m_strHello;
	TestVector m_avVectors[10][5];
};


void TestClass::Save(LoadSave::SaveSystem& ss)
{
	ss.SaveAtom("std::string", &m_strHello, "m_strHello");
	ss.SaveAtom("int", &i1337, "i1337");
	ss.SaveAtom("int", &i23, "i23");
	std::vector<int> dims;
	dims.push_back(10);
	dims.push_back(5);

	ss.OpenObjectArray("TestVector", dims, "avVectors");
	for(int i=0; i<50; i++)
		ss.AddObject((LoadSave::Saveable*)&(m_avVectors[i/5][i%5]));
}

void
TestClass::Load(LoadSave::LoadSystem& ls)
{
	ls.LoadAtom("std::string", &m_strHello);
	ls.LoadAtom("int", &i1337);
	ls.LoadAtom("int", &i23);
	ls.LoadObjectArray(NULL);
	for(int i=0; i<10; i++)
		for(int j=0; j<5; j++)
			m_avVectors[i][j]=*((TestVector*)ls.LoadArrayObject());
}


void main()
{
	RegisterStandardAtoms();
	LoadSave::LoadSave::Instance().RegisterObject(&TestClass::Register);
	LoadSave::LoadSave::Instance().RegisterObject(&TestVector::Register);

	LoadSave::SaveSystem *ss=LoadSave::LoadSave::Instance().CreateSaveFile("bin_test.bin", "xml_test.xml");
	TestClass tc;
	tc.Init();
	ss->SaveObject((LoadSave::Saveable*)&tc, "tc");
	ss->SaveObject((LoadSave::Saveable*)&tc, "tc_ref");
	ss->CloseFiles();
	delete ss;

	TestClass tc2, *tc3;
	LoadSave::LoadSystem *ls=LoadSave::LoadSave::Instance().LoadFile("bin_test.bin");

	tc2=*((TestClass*)ls->LoadObject());
	tc3=(TestClass*)ls->LoadObject();
	ls->CloseFile();
	delete ls;

	void* pData;
	int iDataSize;
	ss=LoadSave::LoadSave::Instance().CreateSaveFile(pData, iDataSize, "");
	ss->SaveObject((LoadSave::Saveable*)&tc, "tc");
	ss->SaveObject((LoadSave::Saveable*)&tc, "tc_ref");
	ss->CloseFiles();
	delete ss;

	ls=LoadSave::LoadSave::Instance().LoadFile(pData, iDataSize);

	tc2=*((TestClass*)ls->LoadObject());
	tc3=(TestClass*)ls->LoadObject();
	ls->CloseFile();
	delete ls;
	delete pData;

}