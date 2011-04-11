
#pragma once

#include "IceIncludes.h"
#include "LoadSave.h"

namespace Ice
{

	template<class T>
	class SaveableVectorHandler : LoadSave::AtomHandler
	{
	public:
		SaveableVectorHandler (std::string name){ m_strName= std::string(name); }
		std::string& TellName(){return m_strName;}

		void Save(LoadSave::SaveSystem& ss, void* pData, std::string strVarName)
		{
			/*test what category the data is*/
			std::vector<int> dims;
			std::vector< std::shared_ptr<T> >* pVector= (std::vector< std::shared_ptr<T> >*)pData;
			dims.push_back(pVector->size());

			ss.OpenObjectArray("Saveable" , dims, "_vector");

			for (auto it=pVector->begin(); it != pVector->end(); it++)
			{
				ss.AddObject((*it).get());
			}
		}
		void Load(LoadSave::LoadSystem& ls, void* pDest)
		{
			std::vector<int> dims;
			std::vector< std::shared_ptr<T> >* pVector=(std::vector< std::shared_ptr<T> >*)pDest;
			std::string str;
			dims=ls.LoadObjectArray(&str);
			if(str!="Saveable")
				return;
			for(int i=0; i<dims[0]; i++)
			{
				pVector->push_back(std::static_pointer_cast<T, LoadSave::Saveable>(ls.LoadArrayObject()));
			}
		}
	private:
		std::string m_strName;
	};

}