

#ifndef __wxPropertyGridListener_H__
#define __wxPropertyGridListener_H__

#include "wx/propgrid/propgrid.h"

class wxPropertyGridListener
{
public:
	virtual ~wxPropertyGridListener() { };
	virtual void OnUpdate() { };
	virtual void OnApply() { };
	virtual void OnActivate() { };
	virtual void OnLeave() { };

	void Init(wxPropertyGrid* propgrid)
	{
		mPropGrid = propgrid;
	}

protected:
	wxPropertyGrid *mPropGrid;
};

#endif