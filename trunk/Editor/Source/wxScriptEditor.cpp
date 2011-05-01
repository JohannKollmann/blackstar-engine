

#include "wxScriptEditor.h"
#include "wxEdit.h"  
#include <list>
#include <iterator>
#include <cstring>

BEGIN_EVENT_TABLE (wxScriptEditor, wxStyledTextCtrl)
	EVT_STC_MARGINCLICK (wxID_ANY,     wxScriptEditor::OnMarginClick)
	EVT_UPDATE_UI(wxID_ANY, 			wxScriptEditor::OnUpdateUI)
    EVT_STC_CHARADDED (wxID_ANY,       wxScriptEditor::OnCharAdded)
	EVT_STC_MODIFIED(wxID_ANY, wxScriptEditor::OnModified)
	EVT_KEY_DOWN(wxScriptEditor::OnKeyPressed)

    EVT_FIND(wxID_ANY, wxScriptEditor::OnFindDialog)
    EVT_FIND_NEXT(wxID_ANY, wxScriptEditor::OnFindDialog)
    EVT_FIND_REPLACE(wxID_ANY, wxScriptEditor::OnFindDialog)
    EVT_FIND_REPLACE_ALL(wxID_ANY, wxScriptEditor::OnFindDialog)
    EVT_FIND_CLOSE(wxID_ANY, wxScriptEditor::OnFindDialog)
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
	SetEOLMode(2);
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

    StyleSetForeground (wxSTC_STYLE_LINENUMBER, wxColour("DARK GREY"));
    StyleSetBackground (wxSTC_STYLE_LINENUMBER, *wxWHITE);
    StyleSetForeground (wxSTC_STYLE_INDENTGUIDE, wxColour("DARK GREY"));

    SetMarginWidth(0, TextWidth(wxSTC_STYLE_LINENUMBER, "_99999"));

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

void wxScriptEditor::OnUpdateUI(wxUpdateUIEvent& key)
{
	//Highlight matching braces
	BraceHighlight(wxSTC_INVALID_POSITION, wxSTC_INVALID_POSITION);
	char c = GetCharAt(GetCurrentPos());
	if (strchr("()[]{}", c))
	{
		int b = BraceMatch(GetCurrentPos());
		if (b != wxSTC_INVALID_POSITION) BraceHighlight(GetCurrentPos(), b);
		else BraceBadLight(GetCurrentPos());
	}
}

void wxScriptEditor::OnKeyPressed(wxKeyEvent& key)
{
	//if CTRL + s
	if (key.ControlDown() && key.GetKeyCode() == 83) {
		SaveScript();
	}
	//if CTRL + " "
	else if (key.ControlDown() && key.GetKeyCode() == 32) {
		IntelliSense(GetCurrentPos());
	}
	//CTRL + f
	else if (key.ControlDown() && key.GetKeyCode() == 70)
	{
		mFindDialog = new wxFindReplaceDialog(this, &mFindReplaceData, "Find");
		mFindDialog->Show();
	}
	//CTRL + h
	else if (key.ControlDown() && key.GetKeyCode() == 72)
	{
		mFindReplaceDialog = new wxFindReplaceDialog(this, &mFindReplaceData, "Find", wxFR_REPLACEDIALOG);
		mFindReplaceDialog->Show();
	}
	else if (key.GetKeyCode() == WXK_F3)
	{
		OnFindDialog(wxFindDialogEvent(wxEVT_COMMAND_FIND_NEXT));
	}
	OnKeyDown(key);
}

void wxScriptEditor::OnFindDialog(wxFindDialogEvent& event)
{
    wxEventType type = event.GetEventType();

    if (type == wxEVT_COMMAND_FIND || type == wxEVT_COMMAND_FIND_NEXT)
    {
		int flags = 0;
		if (mFindReplaceData.GetFlags() & wxFR_WHOLEWORD) flags = flags|wxSTC_FIND_WHOLEWORD;
		if (mFindReplaceData.GetFlags() & wxFR_MATCHCASE) flags = flags|wxSTC_FIND_MATCHCASE;

		long currFrom, currTo;
		GetSelection(&currFrom, &currTo);
		SetSelection(currFrom+1, currTo);
		SearchAnchor();
		SetSelection(currFrom, currTo);
		SearchNext(flags, mFindReplaceData.GetFindString());
		SearchAnchor();
		EnsureCaretVisible();
    }
    else if (type == wxEVT_COMMAND_FIND_REPLACE)
    {
		ReplaceSelection(mFindReplaceData.GetReplaceString());
    }
	else if (type == wxEVT_COMMAND_FIND_REPLACE_ALL)
	{
		long currFrom, currTo;
		GetSelection(&currFrom, &currTo);
		SetSelectionStart(0);
		SetSelectionEnd(GetLength());
		SearchAnchor();
		int start = 0;
		while ((start = SearchNext(0, mFindReplaceData.GetFindString())) > -1)
		{
			ReplaceSelection(mFindReplaceData.GetReplaceString());
			SetSelectionStart(start+1);
			SetSelectionEnd(GetLength());
			SearchAnchor();
		}
		SetSelectionStart(currFrom);
		SetSelectionEnd(currTo);
		SearchAnchor();
	}
    else if (type == wxEVT_COMMAND_FIND_CLOSE)
    {
        wxFindReplaceDialog *dlg = event.GetDialog();
        dlg->Destroy();
		SetFocus();
    }
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
	if (path.Find(".lua") != wxString::npos)
	{
		StyleClearAll();
		SetLexer(wxSTC_LEX_LUA);
	
		//StyleSetBold(14, true);
		for (int i = 0; i < 20; i++)
		{
			wxFont font (10, wxMODERN, wxNORMAL, wxNORMAL);
			StyleSetFont (i, font);
		}

		StyleSetBold(wxSTC_STYLE_BRACELIGHT, true);
		StyleSetForeground(wxSTC_STYLE_BRACELIGHT, wxColour("red"));

		//Keywords
		StyleSetBold(5, true);
		StyleSetForeground(5, wxColour("blue"));
		SetKeyWords(0, ("and break do else elseif end for function if local nil not or repeat return then until while true false"));

		//Strings
		StyleSetForeground(6, wxColour("brown"));
		StyleSetForeground(7, wxColour("brown"));

		//Comments
		StyleSetForeground(1, wxColour("forest green"));
		StyleSetForeground(2, wxColour("forest green"));
		StyleSetForeground(3, wxColour("forest green"));
	}
	else if (path.Find(".material") != wxString::npos || path.Find(".program") != wxString::npos || path.Find(".compositor") != wxString::npos)
	{
		StyleClearAll();
		SetLexer(wxSTC_LEX_CPP);

		StyleSetBold(wxSTC_STYLE_BRACELIGHT, true);
		StyleSetForeground(wxSTC_STYLE_BRACELIGHT, wxColour("red"));
	
		StyleSetForeground(wxSTC_C_STRING,					wxColour(150,0,50));
		StyleSetForeground(wxSTC_C_PREPROCESSOR,			wxColour(50,50,50));
		StyleSetForeground(wxSTC_C_NUMBER,					wxColour(0, 0, 0));
		StyleSetForeground(wxSTC_C_WORD,					wxColour(0, 0, 255));
		StyleSetForeground(wxSTC_C_WORD2,					wxColour(200, 0, 0));
        StyleSetForeground (wxSTC_C_COMMENT,				wxColour("forest green"));
        StyleSetForeground (wxSTC_C_COMMENTLINE,			wxColour("forest green"));
        StyleSetForeground (wxSTC_C_COMMENTDOC,				wxColour("forest green"));
        StyleSetForeground (wxSTC_C_COMMENTDOCKEYWORD,		wxColour("forest green"));
        StyleSetForeground (wxSTC_C_COMMENTDOCKEYWORDERROR, wxColour("forest green"));
        StyleSetBold(wxSTC_C_WORD, false);
        StyleSetBold(wxSTC_C_WORD2, false);
        StyleSetBold(wxSTC_C_COMMENTDOCKEYWORD, false);
		SetKeyWords(0, wxT("compositor material technique pass target abstract import texture_unit vertex_program default_params shared_params shared_params_ref shared_params fragment_program vertex_program_ref fragment_program_ref shadow_caster_vertex_program_ref shadow_receiver_fragment_program_ref shadow_receiver_vertex_program_ref"));
		//visibility_mask first_render_queue last_render_queue shadows target pass input colour_value identifier material_scheme entry_point source
        //SetKeyWords(1, wxT("clear render_scene render_quad blend_diffuse_alpha src_texture src_current colour_blend on off alpha_blend none clamp linear always_pass wrap bilinear trilinear point mirror once_per_light add dest_colour zero replace combinedUVW spherical one separateUV cubic_reflection true false scale_x scale_y sine shadow PF_BYTE_LA Input time_0_x anisotropic true false int half float float2 float3 float4 float3x3 float3x4 float4x3 float4x4 double worldviewproj_matrix ambient_light_colour light_position_object_space light_diffuse_colour custom world_matrix_array_3x4 viewproj_matrix light_specular_colour camera_position_object_space light_position time"));
	}
	else if (path.Find(".hlsl") != wxString::npos || path.Find(".cg") != wxString::npos || path.Find(".glsl") != wxString::npos)
	{
		StyleClearAll();
		SetLexer(wxSTC_LEX_CPP);

		StyleSetBold(wxSTC_STYLE_BRACELIGHT, true);
		StyleSetForeground(wxSTC_STYLE_BRACELIGHT, wxColour("red"));

		StyleSetForeground(wxSTC_C_STRING,					wxColour(200,0,0));
		StyleSetForeground(wxSTC_C_PREPROCESSOR,			wxColour(50,50,50));
		StyleSetForeground(wxSTC_C_NUMBER,					wxColour(0, 0, 0));
		StyleSetForeground(wxSTC_C_WORD,					wxColour(0, 0, 255));
		StyleSetForeground(wxSTC_C_WORD2,					wxColour(200, 0, 0));
        StyleSetForeground (wxSTC_C_COMMENT,				wxColour("forest green"));
        StyleSetForeground (wxSTC_C_COMMENTLINE,			wxColour("forest green"));
        StyleSetForeground (wxSTC_C_COMMENTDOC,				wxColour("forest green"));
        StyleSetForeground (wxSTC_C_COMMENTDOCKEYWORD,		wxColour("forest green"));
        StyleSetForeground (wxSTC_C_COMMENTDOCKEYWORDERROR, wxColour("forest green"));
        StyleSetBold(wxSTC_C_WORD, false);
        StyleSetBold(wxSTC_C_WORD2, false);
        StyleSetBold(wxSTC_C_COMMENTDOCKEYWORD, false);
		SetKeyWords(0, wxT("return for if else while break continue uniform out"));
        SetKeyWords(1, wxT("void const int float float2 float3 float4 float4x3 float3x4 float3x3 void char double sampler2D COLOR"));
	}
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