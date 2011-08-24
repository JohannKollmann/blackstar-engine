#include "IceLeakWatch.h"

namespace Ice
{
	LeakManager&
	LeakManager::getInstance()
	{
		static LeakManager singleton;
		return singleton;
	}

	void
	LeakWatch::addPointer(void* pPointer)
	{
		mPointer=pPointer;
		LeakManager::getInstance().addPointer(this);
	}

	LeakWatch*
	LeakWatch::setFile(std::string strFile)
	{
		m_strFile=strFile;
		return this;
	}

	LeakWatch*
	LeakWatch::setFunction(std::string strFunction)
	{
		m_strFunction=strFunction;
		return this;
	}

	LeakWatch*
	LeakWatch::setLine(int iLine)
	{
		m_iLine=iLine;
		return this;
	}

	const void*
	LeakWatch::getPointer()
	{return mPointer;}
	
	std::string 
	LeakWatch::getDesc()
	{
		std::stringstream out;
		out << m_iLine;
		return std::string("leak in ") + m_strFile + std::string(" @@ ") + m_strFunction + std::string(" l. ") + out.str();
	}

	LeakManager::~LeakManager()
	{
		/*for(std::map<void*, LeakWatch*>::iterator it=m_Pointers.begin(); it!=m_Pointers.end(); it++)
		{
			delete it->first;
			delete it->second;
		}*/
	}
	
	void
	LeakManager::addPointer(LeakWatch* pointer)
	{
		m_Pointers[(void*)pointer->getPointer()]=pointer;
	}
	
	LeakWatch*
	LeakManager::newWatch()
	{return new LeakWatch;}
	
	std::vector<std::string>
	LeakManager::reportLeaks()
	{
		std::vector<std::string> leaks;
		leaks.resize(m_Pointers.size());
		int iLeak=0;
		for(std::map<void*, LeakWatch*>::iterator it=m_Pointers.begin(); it!=m_Pointers.end(); it++)
			leaks[iLeak++]=it->second->getDesc();
		return leaks;
	}
};