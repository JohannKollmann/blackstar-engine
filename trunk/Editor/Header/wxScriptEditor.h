
#pragma once

#include "wx/stc/stc.h"

class wxScriptEditor : public wxStyledTextCtrl
{
private:
	wxString mCurrentFile;

protected:
	DECLARE_EVENT_TABLE()
	void OnCharAdded (wxStyledTextEvent &event);
	void OnMarginClick (wxStyledTextEvent &event);

public:
    wxScriptEditor (wxWindow *parent, wxWindowID id = wxID_ANY,
          const wxPoint &pos = wxDefaultPosition,
          const wxSize &size = wxDefaultSize,
          long style = wxSUNKEN_BORDER|wxVSCROLL
         );
	~wxScriptEditor(void);

	void LoadScript(wxString path);
	void SaveScript();
};
