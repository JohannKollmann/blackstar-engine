
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
			parameters->AddItem(i->keyname, i->defaultVal);
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
			DataMap::Item gp = parameters->GetNext();
			for (auto i = mRefParams.begin(); i != mRefParams.end(); i++)
			{
				if (i->keyname == gp.key)
				{
					IceAssert(i->defaultVal.getType() == gp.data->getType());
					gp.data->Get(i->target);
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
			parameters->AddItem(i->keyname, i->target, i->defaultVal.getType());
		}
	}

}