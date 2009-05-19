/*
Copyright (c) 2008 Erik Beran

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef _CONNEX_NODE_EXT_H
#define _CONNEX_NODE_EXT_H

#include "connexnode.h"

#include "wx/spinbutt.h"
#include "wx/app.h"
#include "wx/frame.h"

class MyApp : public wxApp
{
public:
	virtual bool OnInit();
};

class wxConnexSpace;
class Simple : public wxFrame
{
protected:
	wxConnexSpace* m_pCXSpace;

public:
	Simple( const wxString& title );

};

/*

usage:

wxConnexSpace *m_ConnexSpace = new wxConnexSpace( *parent window/frame whatever* , wxID_ANY, wxPoint(0,0), wxSize(150, 150), wxNO_BORDER);
m_ConnexSpace->SetBackgroundColour( wxColor(128,128,128,255) );
wxConnexNode* el	= new wxConnexNode( m_ConnexSpace, wxID_ANY, wxPoint(25,20), wxSize(150, 50) );
wxConnexNode* el2	= new wxConnexNodeTakesFloat( m_ConnexSpace, wxID_ANY, wxPoint(175,150), wxSize(125, 50) );
wxConnexNode* el3	= new wxConnexNodeFloatParam( m_ConnexSpace, wxID_ANY, wxPoint(75,250), wxSize(110, 250) );

el->SetLabel( "node 0"	);
el2->SetLabel( "node 1"	);
el3->SetLabel( "Float param" );
m_ConnexSpace->AddNode( el );
m_ConnexSpace->AddNode( el2 );
m_ConnexSpace->AddNode( el3 );

*/

class wxConnexNodeFloatParam : public wxConnexNode
{
public:
	wxSpinButton* m_Button;

public:
	wxConnexNodeFloatParam(	wxConnexSpace *parent, wxWindowID id = wxID_ANY,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxSize(50, 50), long style = wxNO_BORDER | wxCAPTION,
		const wxValidator& validator = wxDefaultValidator,
		const wxString& name = wxPanelNameStr );

	void OnSpin( wxSpinEvent &event );
};

class wxConnexNodeTakesFloat: public wxConnexNode
{
public:
	wxConnexNodeTakesFloat(	wxConnexSpace *parent, wxWindowID id = wxID_ANY,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxSize(50, 50), long style = wxNO_BORDER | wxCAPTION,
		const wxValidator& validator = wxDefaultValidator,
		const wxString& name = wxPanelNameStr );

	virtual void OnPortDataUpdate( wxConnexPortBase* port );
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// very basic examples of custom port classes

class wxConnexPortFloat : public wxConnexPortBase
{
public:
	float m_Float;

public:
	wxConnexPortFloat();

	virtual bool CanTakeAsInput( const wxConnexPortClass &port_class );

	virtual void OnInputUpdate( wxConnexPortBase *port );

	DECL_PORT_CLASS();
};

#endif
