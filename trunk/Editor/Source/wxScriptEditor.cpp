
#include "wxScriptEditor.h"
#include "wxEdit.h"


BEGIN_EVENT_TABLE (wxScriptEditor, wxStyledTextCtrl)
    EVT_STC_MARGINCLICK (wxID_ANY,     wxScriptEditor::OnMarginClick)
    EVT_STC_CHARADDED (wxID_ANY,       wxScriptEditor::OnCharAdded)
	EVT_STC_MODIFIED(wxID_ANY, wxScriptEditor::OnModified)
	EVT_KEY_DOWN(wxScriptEditor::OnKeyPressed)
END_EVENT_TABLE()

wxScriptEditor::wxScriptEditor(wxWindow *parent, wxWindowID id,
            const wxPoint &pos,
            const wxSize &size,
            long style)
    : wxStyledTextCtrl (parent, id, pos, size, style)
{
	mOverrideChange = true;

    // default font for all styles
    SetViewEOL (false);
    SetIndentationGuides (false);
    SetEdgeMode (wxSTC_EDGE_NONE);
    SetViewWhiteSpace (wxSTC_WS_INVISIBLE);
    SetOvertype (false);
    SetReadOnly (false);
    SetWrapMode (wxSTC_WRAP_NONE);
    wxFont font (10, wxMODERN, wxNORMAL, wxNORMAL);
    StyleSetFont (wxSTC_STYLE_DEFAULT, font);
    StyleSetForeground (wxSTC_STYLE_DEFAULT, *wxBLACK);
    StyleSetBackground (wxSTC_STYLE_DEFAULT, *wxWHITE);

    StyleSetForeground (wxSTC_STYLE_LINENUMBER, wxColour (_T("DARK GREY")));
    StyleSetBackground (wxSTC_STYLE_LINENUMBER, *wxWHITE);
    StyleSetForeground(wxSTC_STYLE_INDENTGUIDE, wxColour (_T("DARK GREY")));

    SetMarginWidth (0, TextWidth (wxSTC_STYLE_LINENUMBER, _T("_99999")));

	StyleClearAll();
	SetLexer(wxSTC_LEX_LUA);
	
	//StyleSetBold(14, true);
	for (int i = 0; i < 20; i++)
	{
        wxFont font (10, wxMODERN, wxNORMAL, wxNORMAL);
        StyleSetFont (i, font);
	}

	//Keywords
	StyleSetBold(5, true);
	StyleSetForeground(5, wxColour("blue"));
	SetKeyWords(0, wxT("and break do else elseif end for function if local nil not or repeat return then until while"));

	//Strings
	StyleSetForeground(6, wxColour("brown"));
	StyleSetForeground(7, wxColour("brown"));

	//Comments
	StyleSetForeground(1, wxColour("forest green"));
	StyleSetForeground(2, wxColour("forest green"));
	StyleSetForeground(3, wxColour("forest green"));

    SetTabWidth (4);
    SetUseTabs (false);
    SetTabIndents (true);
    SetBackSpaceUnIndents (true);
	SetIndent(4);

    // miscelaneous
    SetLayoutCache (wxSTC_CACHE_PAGE);	
}

wxScriptEditor::~wxScriptEditor(void)
{
}

void wxScriptEditor::OnKeyPressed(wxKeyEvent& key)
{
	if (key.ControlDown() && key.GetKeyCode() == 83) SaveScript();
	OnKeyDown(key);
}

void wxScriptEditor::OnModified(wxStyledTextEvent &event)
{
	if (mOverrideChange) mOverrideChange = false;
	else wxEdit::Instance().GetMainNotebook()->SetModified(this, true);
}

void wxScriptEditor::OnCharAdded(wxStyledTextEvent &event)
{
	char chr = (char)event.GetKey();
	int currentLine = GetCurrentLine();
	if (chr == '\n')
	{
		//Tabhelper

		wxString line;
		if (currentLine > 0) line = GetLine(currentLine-1);
		int tabs = 0;
		for (unsigned int i = 0; i < line.length(); i++)
		{
			char c = line[i];
			if (c != 32) break;
			tabs++;
		}
		tabs = tabs / 4;
		for (int i = 0; i < tabs; i++)  CmdKeyExecute (wxSTC_CMD_TAB);

		int dos = line.Find("do");
		int thens = line.Find("then");
		int elses = line.Find("else");
		int elifs = line.Find("elif");
		int length = line.length();
		if ((dos == length - 4 && dos != wxString::npos)
			|| (thens == length - 6 && thens != wxString::npos)
			|| (elses == length - 6 && elses != wxString::npos)
			|| (elifs == length - 6 && elifs != wxString::npos)
			|| line.Find("function") != wxString::npos)
				CmdKeyExecute (wxSTC_CMD_TAB);
	}

}
void wxScriptEditor::OnMarginClick (wxStyledTextEvent &event)
{
}

void wxScriptEditor::LoadScript(wxString path)
{
	mCurrentFile = path;
	LoadFile(path);
	mOverrideChange = true;
}

void wxScriptEditor::SaveScript()
{
	if (mCurrentFile == "") return;
	SaveFile(mCurrentFile);
	wxEdit::Instance().GetMainNotebook()->SetModified(this, false);

}