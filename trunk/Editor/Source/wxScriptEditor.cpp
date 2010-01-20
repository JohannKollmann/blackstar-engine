
#include "wxScriptEditor.h"
#include "wxEdit.h"  
#include <list>
#include <iterator>

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
    StyleSetForeground (wxSTC_STYLE_INDENTGUIDE, wxColour (_T("DARK GREY")));

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
	SetKeyWords(0, wxT("and break do else elseif end for function if local nil not or repeat return then until while true false"));

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
	//if CTRL + "s"
	if (key.ControlDown() && key.GetKeyCode() == 83) {
		SaveScript();
	}
	//if CTRL + " "
	else if (key.ControlDown() && key.GetKeyCode() == 32) {
		IntelliSense(GetCurrentPos());
	}
	OnKeyDown(key);
}

void wxScriptEditor::OnModified(wxStyledTextEvent &event)
{
	if (mOverrideChange) mOverrideChange = false;
	else if (
		(event.GetModificationType() & wxSTC_MOD_INSERTTEXT) == wxSTC_MOD_INSERTTEXT
		|| (event.GetModificationType() & wxSTC_MOD_DELETETEXT) == wxSTC_MOD_DELETETEXT)
	{
		wxEdit::Instance().GetMainNotebook()->SetModified(this, true);
	}
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
		int realtabs = 0;
		for (unsigned int i = 0; i < line.length(); i++)
		{
			char c = line[i];
			if (c != 32 && c != 9) break;
			if (c == 32) tabs++;
			if (c == 9) realtabs++;
		}
		tabs = (tabs / 4) + realtabs;
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

#define ISWHITE(c)(c==' ' || c=='\t' || c=='\n' || c=='\r')

#define ISSYNTAX(c)(c=='(' || c==')' || c=='.' || c=='+' || c=='-' || c=='/'  || c=='*' || c==';' || c=='~' || c=='='  || c=='>' || c=='<' || c==',' || c=='[' || c==']')

#define ISCOMMAND(s)(s.compare("function")==0 || s.compare("for")==0 || s.compare("while")==0 ||\
						s.compare("if")==0 || s.compare("then")==0 || s.compare("end")==0 ||\
						s.compare("true")==0 || s.compare("false")==0 || s.compare("elseif")==0 ||\
						s.compare("not")==0)

void wxScriptEditor::IntelliSense(int currentPosition)
{		
	int nCharsEntered = 0;
	while(!ISWHITE(GetCharAt(currentPosition-nCharsEntered-1)) && !ISSYNTAX(GetCharAt(currentPosition-nCharsEntered-1)))
		nCharsEntered++;

	unsigned int iSkipIndex=currentPosition-nCharsEntered;

	wxString text = GetText();

	//split the source into bits, removing unnecessary keywords
	std::map<wxString, int> mScriptStrings;
	wxString strCurrString;
	bool bIsString=false;
	bool bIsFunction=false;
	for(unsigned int iIndex=0; iIndex<text.length(); iIndex++)
	{
		if(iIndex>=iSkipIndex && iIndex<iSkipIndex+nCharsEntered)
			continue;
		if(text[iIndex]=='\"')
			bIsString=!bIsString;
		if((ISSYNTAX(text[iIndex]) || ISWHITE(text[iIndex]))&& !bIsString)
		{
			bIsString=false;
			if(strCurrString.length())
			{
				if(ISCOMMAND(strCurrString))
				{
					strCurrString.clear();
					continue;
				}
				//check if this is a function. search for an opening bracket
				unsigned int iTestChar=iIndex;
				for(; iTestChar<text.length();iTestChar++)
				{
					wxChar c=text[iTestChar];
					if(!ISWHITE(c))
						break;
				}
				//while(ISWHITE(text[++iTestChar]));

				if(text[iTestChar]=='(')//bracket found. it's a function
					bIsFunction=true;
				if(bIsFunction)
					mScriptStrings[strCurrString]=1;
				else
				{
					if(mScriptStrings.find(strCurrString)==mScriptStrings.end())
						mScriptStrings[strCurrString]=0;
				}
				bIsFunction=false;
				strCurrString.clear();
			}
		}
		else
			strCurrString+=text[iIndex];
	}

	//find functions
	/*
	int iOffset=0;
	wxString strFn
	while ((iOffset=text.find("function", lastStart))!=-1)
	{
		//skip white space
		while(iOffset+
		int length = text.find("(", start) - start;
		if (start < lastStart) {
			//if all founded.
			stillSearch = false;
		} else {
			funcNamesList.push_back(text.substr(start, length) + "()");
			lastStart = start;
		}
	}
	funcNamesList.sort();

	//transform the list into a string for AutoCompShow.
	wxString funcNames;
	for (std::list<wxString>::iterator i =  funcNamesList.begin(); i != funcNamesList.end(); ++i) {
		funcNames += *i + " ";
	}*/

	wxString strTips;
	for (std::map<wxString, int>::const_iterator it =  mScriptStrings.begin(); it != mScriptStrings.end(); ++it)
		strTips += (it->first + wxString(it->second==1 ? "(" : "") + wxString(" "));

	AutoCompSetChooseSingle(0);
	AutoCompShow(nCharsEntered, strTips);

}