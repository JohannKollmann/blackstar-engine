
#include "IceGOCEditorInterface.h"

namespace Ice
{
	void GOCStaticEditorInterface::GetDefaultParameters(DataMap *parameters)
	{
		if (!mInitedRefParams)
		{
			_initRefParams();
			mInitedRefParams = true;
		}

		for (auto i = mRefParams.begin(); i != mRefParams.end(); i++)
		{
			parameters->AddProperty(i->defaultVal);
		}
	}
	void GOCStaticEditorInterface::SetParameters(DataMap *parameters)
	{
		if (!mInitedRefParams)
		{
			_initRefParams();
			mInitedRefParams = true;
		}

		while (parameters->HasNext())
		{
			GenericProperty gp = parameters->GetNext();
			for (auto i = mRefParams.begin(); i != mRefParams.end(); i++)
			{
				if (i->defaultVal.mKey == gp.mKey)
				{
					assert(i->defaultVal.getType() == gp.getType());
					gp.Get(i->target);
					break;
				}
			}
		}

		OnSetParameters();
	}
	void GOCStaticEditorInterface::GetParameters(DataMap *parameters)
	{
		if (!mInitedRefParams)
		{
			_initRefParams();
			mInitedRefParams = true;
		}

		for (auto i = mRefParams.begin(); i != mRefParams.end(); i++)
		{
			GenericProperty::PropertyTypes type = i->defaultVal.getType();
			parameters->AddValue(i->defaultVal.mKey, i->target, i->defaultVal.getType());
		}
	}

}