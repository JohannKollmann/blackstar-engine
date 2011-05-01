
#pragma once

#include "wx/stc/stc.h"
#include <list>
#include "EDTIncludes.h"
#include <wx/fdrepdlg.h>


class wxScriptEditor : public wxStyledTextCtrl
{
private:
	wxString mCurrentFile;
	bool mOverrideChange;
	wxFindReplaceDialog *mFindDialog;
	wxFindReplaceDialog *mFindReplaceDialog;
	wxFindReplaceData mFindReplaceData;

protected:
	DECLARE_EVENT_TABLE()
	void OnCharAdded (wxStyledTextEvent &event);
	void OnUpdateUI (wxUpdateUIEvent &event);
	void OnMarginClick (wxStyledTextEvent &event);
	void OnModified (wxStyledTextEvent &event);
	void OnKeyPressed(wxKeyEvent& key);
	void OnFindDialog(wxFindDialogEvent& event);

public:
    wxScriptEditor (wxWindow *parent, wxWindowID id = wxID_ANY,
          const wxPoint &pos = wxDefaultPosition,
          const wxSize &size = wxDefaultSize,
          long style = wxSUNKEN_BORDER|wxVSCROLL|wxSTC_STYLE_BRACELIGHT
         );
	~wxScriptEditor(void);

	void LoadScript(wxString path);
	void SaveScript();
	void IntelliSense(int currentPosition);
};
