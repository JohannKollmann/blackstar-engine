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

#include "connex_example.h"
#include "connex.h"
#include "connexspace.h"

bool MyApp::OnInit()
{
	Simple *simple = new Simple(wxT("Simple"));
	simple->Show(true);

	return true;
}

Simple::Simple(const wxString& title)
: wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(500, 500))
{
	m_pCXSpace = new wxConnexSpace( this, wxID_ANY, wxPoint(0,0), wxSize(150, 150), wxNO_BORDER);
	m_pCXSpace->SetBackgroundColour( wxColor(128,128,128,255) );
	wxConnexNode* el	= new wxConnexNode( m_pCXSpace, wxID_ANY, wxPoint(25,20), wxSize(150, 50) );
	wxConnexNode* el2	= new wxConnexNodeTakesFloat( m_pCXSpace, wxID_ANY, wxPoint(175,150), wxSize(125, 50) );
	wxConnexNode* el3	= new wxConnexNodeFloatParam( m_pCXSpace, wxID_ANY, wxPoint(75,200), wxSize(125, 250) );

	el->SetLabel( "node 0"	);
	el2->SetLabel( "node 1"	);
	el3->SetLabel( "Float param" );
	m_pCXSpace->AddNode( el );
	m_pCXSpace->AddNode( el2 );
	m_pCXSpace->AddNode( el3 );


	Centre();
}

///

DEF_PORT_CLASS( wxConnexPortFloat, wxConnexPortBase )

wxConnexNodeFloatParam::wxConnexNodeFloatParam(	wxConnexSpace *parent, wxWindowID id,
						  const wxPoint& pos,
						  const wxSize& size, long style,
						  const wxValidator& validator,
						  const wxString& name)
	: wxConnexNode( parent, id, pos, size, style, validator, name )
{
	wxConnexPortFloat* port = new wxConnexPortFloat();
	port->m_Placement = wxConnexPortBase::PLACE_RIGHT;
	port->SetDirection( wxConnexPortBase::DIR_OUTPUT );
	port->m_Rect.SetPosition( wxPoint( size.x - 10, 16 ) );

	this->AddCXPort( port );

	m_Button = new wxSpinButton( this, wxID_ANY, wxPoint( 10, 16), wxSize( 16, 16 ) );
	m_Button->SetRange(0, 1000);
	m_Button->SetValue( 500 );

	Connect( m_Button->GetId(), wxEVT_SCROLL_THUMBTRACK, 
		wxSpinEventHandler(wxConnexNodeFloatParam::OnSpin) );
}

void wxConnexNodeFloatParam::OnSpin( wxSpinEvent &event )
{
	int val = m_Button->GetValue();

	SetLabel( wxString("Output: ", wxConvUTF8) + wxString::Format(wxT("%f"), (float)val / 1000.0f) );

	((wxConnexPortFloat*)m_Ports.front())->m_Float = (float)val / 1000.0f;
	m_Ports.front()->RefreshConnections();
	Refresh();
}

//////////////////////////////////////////////////////////////////////////

wxConnexNodeTakesFloat::wxConnexNodeTakesFloat(	wxConnexSpace *parent, wxWindowID id,
											   const wxPoint& pos,
											   const wxSize& size, long style,
											   const wxValidator& validator,
											   const wxString& name)
	: wxConnexNode( parent, id, pos, size, style, validator, name )
{
	wxConnexPortFloat* port = new wxConnexPortFloat();
	port->m_Placement = wxConnexPortBase::PLACE_BOTTOM;
	port->SetDirection( wxConnexPortBase::DIR_INPUT );
	port->m_Rect.SetPosition( wxPoint( size.GetWidth() / 2, size.GetHeight() - port->m_Rect.GetHeight() ) );

	this->AddCXPort( port );
}

void wxConnexNodeTakesFloat::OnPortDataUpdate( wxConnexPortBase* port )
{
	SetLabel( wxString("Input: ", wxConvUTF8) + wxString::Format(wxT("%f"), ((wxConnexPortFloat*)port)->m_Float) ); 

	Refresh();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

wxConnexPortFloat::wxConnexPortFloat()
: wxConnexPortBase()
{
	m_Float = 0.0f;
}

bool wxConnexPortFloat::CanTakeAsInput( const wxConnexPortClass &port_class )
{
	if( ms_PortClass.m_RunTimeID == port_class.m_RunTimeID )
		return true;

	return false;
}

void wxConnexPortFloat::OnInputUpdate( wxConnexPortBase *port )
{
	if( !IsInput() )
		return;

	if( port->GetPortClass()->m_RunTimeID == GetPortClass()->m_RunTimeID )
	{
		m_Float = ((wxConnexPortFloat*)port)->m_Float;
	}
}
