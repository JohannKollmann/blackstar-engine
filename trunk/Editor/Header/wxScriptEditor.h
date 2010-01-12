
#pragma once

#include "wx/stc/stc.h"
#include <list>

class wxScriptEditor : public wxStyledTextCtrl
{
private:
	wxString mCurrentFile;
	bool mOverrideChange;

protected:
	DECLARE_EVENT_TABLE()
	void OnCharAdded (wxStyledTextEvent &event);
	void OnMarginClick (wxStyledTextEvent &event);
	void OnModified (wxStyledTextEvent &event);
	void OnKeyPressed(wxKeyEvent& key);

public:
    wxScriptEditor (wxWindow *parent, wxWindowID id = wxID_ANY,
          const wxPoint &pos = wxDefaultPosition,
          const wxSize &size = wxDefaultSize,
          long style = wxSUNKEN_BORDER|wxVSCROLL
         );
	~wxScriptEditor(void);

	void LoadScript(wxString path);
	void SaveScript();
	void IntelliSense(int currentPosition);
};
