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

		LeakWatch* setFile(std::string strFile)
		{
			m_strFile=strFile;
			return this;
		}
		LeakWatch* setFunction(std::string strFunction)
		{
			m_strFunction=strFunction;
			return this;
		}
		LeakWatch* setLine(int iLine)
		{
			m_iLine=iLine;
			return this;
		}
		const void* getPointer(){return mPointer;}
	
		std::string getDesc()
		{
			std::stringstream out;
			out << m_iLine;
			return std::string("leak in ") + m_strFile + std::string(" @@ ") + m_strFunction + std::string(" l. ") + out.str();
		}
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
				delete it->first;
				delete it->second;
				m_Pointers.erase(it);
			}
			//else DebugBreak();
		}

		LeakManager::~LeakManager()
		{
			/*for(std::map<void*, LeakWatch*>::iterator it=m_Pointers.begin(); it!=m_Pointers.end(); it++)
			{
				delete it->first;
				delete it->second;
			}*/
		}
	
		static LeakManager& getInstance();
	
		void addPointer(LeakWatch* pointer)
		{
			m_Pointers[(void*)pointer->getPointer()]=pointer;
		}
	
		static LeakWatch* newWatch(){return new LeakWatch;}
	
		std::vector<std::string> reportLeaks()
		{
			std::vector<std::string> leaks;
			leaks.resize(m_Pointers.size());
			int iLeak=0;
			for(std::map<void*, LeakWatch*>::iterator it=m_Pointers.begin(); it!=m_Pointers.end(); it++)
				leaks[iLeak++]=it->second->getDesc();
			return leaks;
		}
	
	private:
		std::map<void*, LeakWatch*> m_Pointers;
	};
};