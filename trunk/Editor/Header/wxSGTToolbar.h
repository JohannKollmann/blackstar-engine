
#pragma once

#include "wx/toolbar.h"

class wxSGTToolbar : public wxToolBar
{
protected:
	DECLARE_EVENT_TABLE()

	void OnToolEvent(wxCommandEvent &event);

public:
	wxSGTToolbar(wxWindow *parent);
	~wxSGTToolbar(void);
};
