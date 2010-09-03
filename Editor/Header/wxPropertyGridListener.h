

#pragma once

#include "wx/wx.h"
#include "wx/propgrid/propgrid.h"

class wxPropertyGridListener
{
public:
	virtual ~wxPropertyGridListener() { };
	virtual void OnUpdate() { };
	virtual void OnApply() { };
	virtual void OnActivate() { };
	virtual void OnLeave() { };
	virtual bool OnDropText(const wxString& text) { return false; };

	void Init(wxPropertyGrid* propgrid)
	{
		mPropGrid = propgrid;
	}

protected:
	wxPropertyGrid *mPropGrid;
};