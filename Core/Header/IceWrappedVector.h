
#pragma once

#include <vector>

namespace Ice
{
	/**
	* Wraps a std::vector and adds an intern iterator mechanism that support remove of an elements while iterating.
	*/

	template<class T>
	class WrappedVector
	{
	private:

		std::vector<T> mVector;
		unsigned int mCurrentIndex;

	public:
		WrappedVector() : mCurrentIndex(0) {}
		~WrappedVector() {}

		std::vector<T>* GetVector() { return &mVector; }

		void Remove(unsigned int index)
		{
			if (index <= mCurrentIndex) mCurrentIndex--;
			mVector.erase(mVector.begin()+index);
		}
		void Init()
		{
			mCurrentIndex = 0;
		}
		bool HasNext()
		{
			return (mCurrentIndex < mVector.size());
		}
		T& GetNext()
		{
			return mVector[mCurrentIndex++];
		}
	};

}