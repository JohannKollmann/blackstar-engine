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

#include "connexnode.h"
#include "connex.h"
#include "connexspace.h"

IMPLEMENT_CLASS( wxConnexNode, wxWindow )

BEGIN_EVENT_TABLE( wxConnexNode, wxWindow )
	EVT_PAINT( wxConnexNode::OnPaint )
	EVT_MOUSE_EVENTS( wxConnexNode::OnMouseEvent )
END_EVENT_TABLE()

wxConnexNode::wxConnexNode()
: wxWindow()
{
	m_FocusPort = NULL;
}

wxConnexNode::wxConnexNode(	wxConnexSpace *parent, wxWindowID id,
							const wxPoint& pos,
							const wxSize& size, long style,
							const wxValidator& validator,
							const wxString& name )
: wxWindow( parent, id, pos, size, style, name )
{
	m_Space			= parent;
	m_NonScaleSize	= size;
	m_FocusPort		= NULL;
}

void wxConnexNode::AddCXPort( wxConnexPortBase *port )
{
	if( !port )
		return;

	port->m_Parent = this;

	m_Ports.push_front( port );
}

wxConnexPortBase* wxConnexNode::GetCXPort( const wxPoint &client_pos )
{
	PortListIterator port_iter( m_Ports.begin() );

	while( port_iter != m_Ports.end() )
	{
		if( (*port_iter)->m_Rect.Contains( client_pos ) )
			return *port_iter;

		port_iter++;
	}

	return NULL;
}

void wxConnexNode::OnPaint( wxPaintEvent& event )
{
	wxPaintDC dc(this);

	static wxPen pen_lines( wxColour(255,255,255, wxALPHA_TRANSPARENT), 1, wxSOLID );
	static wxPen pen_black( wxColour(0,0,0, wxALPHA_OPAQUE), 1, wxSOLID );
	static wxBrush brush_black( wxColour(0,0,0, wxALPHA_OPAQUE) );
	static wxBrush brush_label( wxColour(200,200,200, wxALPHA_OPAQUE), wxSOLID );

	wxRect rect = GetRect();
	rect.x = 8;
	rect.width -= 16;
	rect.y = 1;
	rect.height -= 10;
	dc.SetPen( pen_lines );
	dc.DrawRoundedRectangle( rect, 5 );

	//not sure if this is a correct way to use this flag :/
	if( GetWindowStyle() & wxCAPTION )
	{
		long height = 16;
		dc.GetTextExtent( GetLabel(), NULL, &height );

		rect.height = height;
		dc.SetBrush( brush_label );
		dc.DrawRoundedRectangle( rect, 5 );
		dc.DrawLabel( GetLabel(), rect, wxALIGN_CENTER );
	}

	PortListIterator port_iter( m_Ports.begin() );

	while( port_iter != m_Ports.end() )
	{
		(*port_iter)->Paint( dc, wxPoint(1, 0) );
		port_iter++;
	}

#ifdef WIN32
	wxWindow::OnPaint( event );
#endif
}

void wxConnexNode::OnMouseEvent( wxMouseEvent &event )
{
	if( event.GetEventType() == wxEVT_LEFT_DOWN )
	{
		m_ClickPoint				= event.GetPosition();
		m_Space->m_FocusNode		= this;
		if( (m_FocusPort = GetCXPort( event.GetPosition() )) )
		{
			m_Space->m_MouseMode		= wxConnexSpace::MOUSE_CONNECT;
			m_Space->m_MouseFromPlace	= m_FocusPort->m_Placement;
		}
		else
		{
			m_Space->m_MouseMode	= wxConnexSpace::MOUSE_DRAG;
			Raise();
		}
	}
	else if( event.GetEventType() == wxEVT_LEFT_UP )
	{
		if( m_Space->m_FocusNode
			&& m_Space->m_FocusNode != this
			&& (m_FocusPort = GetCXPort( event.GetPosition() )) )
		{
			m_Space->ConnectPorts( m_Space->m_FocusNode->m_FocusPort, m_FocusPort );
		}

		m_Space->m_FocusNode		= NULL;
		GetParent()->Refresh();
	}
	else if( event.GetEventType() == wxEVT_MOTION && event.ButtonIsDown( wxMOUSE_BTN_LEFT ) && event.Dragging() )
	{
		if (m_Space->m_MouseMode == wxConnexSpace::MOUSE_CONNECT)
		{
			wxConnexPortBase* port = GetCXPort( event.GetPosition() );
			if( port && m_Space->m_FocusNode != this )
			{
				wxRect mrect = port->m_Rect;
				mrect.x += mrect.width / 2;
				mrect.y += mrect.height / 2;
				m_Space->m_LastMousePos		= mrect.GetPosition() + GetPosition();
				m_Space->m_MouseToPlace		= port->m_Placement;
			}
			else
			{
				m_Space->m_LastMousePos = event.GetPosition() + GetPosition();
			}

			GetParent()->Refresh();
		}
		else if( m_Space->m_FocusNode == this )
		{
			int x = event.m_x + GetPosition().x - m_ClickPoint.x;
			int y = event.m_y + GetPosition().y - m_ClickPoint.y;

			wxPoint pos = GetPosition();
			pos.x = x;
			pos.y = y;

			SetPosition(pos);
			GetParent()->Refresh();
		}
	}
}
