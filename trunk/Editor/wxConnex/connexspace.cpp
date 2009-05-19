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

#include "connexspace.h"
#include "connexnode.h"
#include "connex.h"

IMPLEMENT_CLASS( wxConnexSpace, wxWindow )

BEGIN_EVENT_TABLE( wxConnexSpace, wxWindow )
	EVT_PAINT( wxConnexSpace::OnPaint )
	EVT_MOUSE_EVENTS( wxConnexSpace::OnMouseEvent )
END_EVENT_TABLE()

wxConnexSpace::wxConnexSpace()
: wxWindow()
{
	m_FocusNode				= NULL;
	m_DrawConnectionShadows	= true;
	m_MouseToPlace			= wxConnexPortBase::PLACE_LEFT;
	m_MouseFromPlace		= wxConnexPortBase::PLACE_LEFT;
}

wxConnexSpace::wxConnexSpace(	wxWindow *parent, wxWindowID id,
								const wxPoint& pos,
								const wxSize& size, long style,
								const wxValidator& validator,
								const wxString& name )
	: wxWindow( parent, id, pos, size, style, name )
{
	m_FocusNode				= NULL;
	m_DrawConnectionShadows	= true;
	m_MouseToPlace			= wxConnexPortBase::PLACE_LEFT;
	m_MouseFromPlace		= wxConnexPortBase::PLACE_LEFT;
}

void wxConnexSpace::AddNode( wxConnexNode* node )
{
	m_NodeList.push_front( node );
}

void wxConnexSpace::ConnectPorts( wxConnexPortBase* port0, wxConnexPortBase* port1	)
{
	if( !port0 || !port1 || (port0->IsInput() == port1->IsInput()) )
		return;

	bool make_connection = false;

	wxConnexPortBase* inputp;
	wxConnexPortBase* outputp;

	if( port0->IsInput() )
	{
		inputp	= port0;
		outputp	= port1;
		if( port0->CanTakeAsInput( *port1->GetPortClass() ) )
		{
			make_connection = true;
		}
	}
	else
	{
		inputp	= port1;
		outputp	= port0;
		if( port1->CanTakeAsInput( *port0->GetPortClass() ) )
		{
			make_connection = true;
		}
	}

	if( make_connection )
	{
		inputp->m_Connections.clear();
		inputp->m_Connections.push_back( outputp );
		outputp->m_Connections.push_back( inputp );

		inputp->RefreshConnections();
	}
}

void wxConnexSpace::RemoveConnection( wxConnexPortBase *port )
{
	if( !port )
		return;

	port->RemoveAllConnections();
}

void wxConnexSpace::OnPaint( wxPaintEvent& event )
{
	static wxPen pen_lines( wxColour(0,0,0), 2, wxSOLID );
	static wxPen pen_shadow( wxColour(82,82,82), 2, wxSOLID );
	static wxPen pen_red( wxColour(255,0,0), 3, wxSOLID );

#ifdef WIN32
	wxWindow::OnPaint( event );
#endif

	static wxPoint points[4];
	static wxPoint shadow[4];

	wxPaintDC dc(this);

	int init_pass = 0;
	if( !m_DrawConnectionShadows )
		init_pass = 1;

	//pass 0 = connection spline shadows
	//pass 1 = diffuse connection splines
	int draw_pass;
	for( draw_pass = init_pass; draw_pass < 2; draw_pass++ )
	{
		if( draw_pass == 0 )
			dc.SetPen( pen_shadow );
		else
			dc.SetPen( pen_lines );

		NodeListIterator iter(m_NodeList.begin());
		//iterate through all the nodes
		while( iter != m_NodeList.end() )
		{
			wxPoint node_pos = (*iter)->GetPosition();

			PortListIterator port_iter = (*iter)->m_Ports.begin();
			//iterate through all the nodes ports
			while( port_iter != (*iter)->m_Ports.end() )
			{
				//render only the output ports connections
				if( !(*port_iter)->IsInput() && (*port_iter)->HasConnections())
				{
					PortListIterator conn_iter = (*port_iter)->m_Connections.begin();
					//iterate through all the output ports connections and render them
					while( conn_iter != (*port_iter)->m_Connections.end() )
					{
						wxSize shift = (*port_iter)->m_Rect.GetSize();
						shift.x /= 2;
						shift.y /= 2;
						points[0] = node_pos + (*port_iter)->m_Rect.GetPosition() + shift;
						points[1] = points[0] + wxConnexSplineDelta[(*port_iter)->m_Placement];

						shift = (*conn_iter)->m_Rect.GetSize();
						shift.x /= 2;
						shift.y /= 2;

						points[3] = (*conn_iter)->m_Parent->GetPosition() + (*conn_iter)->m_Rect.GetPosition() + shift;
						points[2] = points[3] + wxConnexSplineDelta[(*conn_iter)->m_Placement];

						if( draw_pass == 0 )
						{
							points[1].x += 3;
							points[2].x += 3;
						}

						dc.DrawSpline(4, points);

						conn_iter++;
					}
				}

				port_iter++;
			}

			iter++;
		}
	}
	

	if( m_MouseMode == MOUSE_CONNECT )
	{
		//render the output to mouse pointer connetion (usually red)
		if( m_FocusNode && m_FocusNode->m_FocusPort )
		{
			wxPoint pos			= m_FocusNode->m_FocusPort->m_Rect.GetPosition();
			wxSize	half_size	= m_FocusNode->m_FocusPort->m_Rect.GetSize();
			half_size.x /= 2;
			half_size.y /= 2;
			points[0] = m_FocusNode->GetPosition() + pos + half_size;
			points[1] = points[0] + wxConnexSplineDelta[m_MouseFromPlace];
			points[3] = m_LastMousePos;
			points[2] = points[3] + wxConnexSplineDelta[m_MouseToPlace];


			if( m_DrawConnectionShadows )
			{
				shadow[0] = points[0];
				shadow[1] = points[1] + wxPoint(3,0);
				shadow[2] = points[2] + wxPoint(3,0);
				shadow[3] = points[3];
				dc.SetPen( pen_shadow );
				dc.DrawSpline( 4, shadow );
			}

			dc.SetPen(pen_red);
			dc.DrawSpline(4, points);
		}
	}
}


void wxConnexSpace::OnMouseEvent( wxMouseEvent& event )
{
	int delta = event.GetWheelRotation();

	if( event.GetEventType() == wxEVT_LEAVE_WINDOW )
	{
		/*
		wxWindow *fcs = FindFocus();
		if( fcs && !GetWindowChild( fcs->GetId() ) )
		{
			if( m_FocusNode )
			{
				//m_FocusNode = NULL;
			}
			//
		}
		*/
	}

	if( event.GetEventType() == wxEVT_LEFT_UP )
	{
		if( m_MouseMode == MOUSE_CONNECT)
		{
			int smurf = 0;
			if( m_FocusNode && m_FocusNode->m_FocusPort )
			{
				RemoveConnection( m_FocusNode->m_FocusPort );
			}
		}
		else
		{
		}
		Refresh();
		m_FocusNode = NULL;
	}

	if( event.GetEventType() == wxEVT_MOTION )
	{
		m_LastMousePos = event.GetPosition();
	}

	if( event.GetEventType() == wxEVT_MOTION && event.ButtonIsDown( wxMOUSE_BTN_LEFT ) && event.Dragging() )
	{
		if( m_FocusNode )
		{
			if( m_MouseMode == MOUSE_CONNECT)
			{
				Refresh();
			}
			else
			{
				wxPoint pos = event.GetPosition() - m_FocusNode->m_ClickPoint;
				m_FocusNode->SetPosition( pos );
			}
		}
	}
}
