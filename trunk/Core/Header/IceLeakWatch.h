#pragma once
#include <map>
#include <vector>
#include <string>
#include <sstream>

#ifdef _DEBUG
	#define ICE_NEW *(Ice::LeakManager::newWatch()->setFile(__FILE__)->setFunction(__FUNCTION__)->setLine(__LINE__))= new
	#define ICE_DELETE Ice::LeakManager::getInstance()=
#else
	#define ICE_NEW new
	#define ICE_DELETE delete
#endif

namespace Ice
{
	
	class __declspec(dllexport) LeakWatch
	{
	public:
		template<class T> inline T* operator =(T* pPointer)
		{
			addPointer((void*) pPointer);
			return pPointer;
		}

		LeakWatch* setFile(std::string strFile);
		LeakWatch* setFunction(std::string strFunction);
		LeakWatch* setLine(int iLine);
		const void* getPointer();
		std::string getDesc();
	private:
		void* mPointer;
		std::string m_strFile, m_strFunction;
		int m_iLine;

		void addPointer(void* pPointer);
	};


	class __declspec(dllexport) LeakManager
	{
	public:
		template<class T> void operator =(T* pPointer)
		{
			std::map<void*, LeakWatch*>::iterator it=m_Pointers.find((void*)pPointer);
			if(it!=m_Pointers.end())
			{
				//delete it->first;
				delete it->second;
				m_Pointers.erase(it);
			}
			//else IceWarning("Could not find pointer address")
			delete pPointer;
		}

		LeakManager::~LeakManager();
		static LeakManager& getInstance();
		static LeakWatch* newWatch();

		void addPointer(LeakWatch* pointer);
	
		std::vector<std::string> reportLeaks();
	
	private:
		std::map<void*, LeakWatch*> m_Pointers;
	};
};