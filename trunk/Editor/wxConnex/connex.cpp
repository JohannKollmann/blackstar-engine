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

#include "connex.h"
#include "connexnode.h"
#include <sstream>

wxPoint wxConnexInArrows[4][3] = {
{ wxPoint(1, 1), wxPoint(7, 1), wxPoint(4, 7) }, //top
{ wxPoint(1, 6), wxPoint(7, 6), wxPoint(4, 0) }, //bottom
{ wxPoint(2, 1), wxPoint(8, 4), wxPoint(2, 7) }, //left
{ wxPoint(7, 1), wxPoint(1, 4), wxPoint(7, 7) }	 //right
};

wxPoint wxConnexSplineDelta[4] = {
	wxPoint(0, -25),	//top
	wxPoint(0, 25),		//bottom
	wxPoint(-25, 0),	//left
	wxPoint(25, 0)		//right
};


unsigned int wxConnexPortBase::ms_NextPortID = 0;

DEF_PORT_CLASS_NO_BASE( wxConnexPortBase )

wxConnexPortBase::wxConnexPortBase()
{
	m_ID			= ++ms_NextPortID;
	m_Rect			= wxRect(0, 0, 8, 8);
	m_Direction		= DIR_INPUT;
	m_Placement		= PLACE_LEFT;
	m_Parent		= NULL;
}

wxConnexPortBase::wxConnexPortBase( PORT_DIR dir, PLACEMENT place, unsigned int id )
{
	if( id == INVALID_PORT_ID )
		id = ++ms_NextPortID;

	m_ID = id;
	m_Rect			= wxRect(0, 0, 10, 10);
	m_Direction		= dir;
	m_Placement		= place;
	m_Parent		= NULL;
}

bool wxConnexPortBase::IsConnected( wxConnexPortBase* port )
{
	if( !port )
		return false;

	PortListIterator iter( m_Connections.begin() );

	while( iter != m_Connections.end() )
	{
		if( *iter == port )
			return true;

		iter++;
	}

	return false;
}

void wxConnexPortBase::RemoveConnection( wxConnexPortBase *port )
{
	if( !port )
		return;

	port->m_Connections.remove( this );
	m_Connections.remove( port );
}

void wxConnexPortBase::RemoveAllConnections()
{
	while( !m_Connections.empty() )
	{
		m_Connections.front()->m_Connections.remove( this );
		m_Connections.pop_front();
	}
}

void wxConnexPortBase::RefreshConnections()
{
	PortListIterator iter( m_Connections.begin() );

	while( iter != m_Connections.end() )
	{
		if( IsInput() )
		{
			//update my input port
			OnInputUpdate( *iter );
			//notify my parent of the change
			m_Parent->OnPortDataUpdate( this );
			//could break here
		}
		else
		{
			//update its input port
			(*iter)->OnInputUpdate( this );
			//notify its parent of the change
			(*iter)->m_Parent->OnPortDataUpdate( *iter );
		}

		iter++;
	}
}

void wxConnexPortBase::Paint( wxPaintDC &dc, wxPoint normal )
{
	static wxPen pen_white( wxColour(255,255,255, wxALPHA_TRANSPARENT), 1, wxSOLID );
	static wxPen pen_black( wxColour(0,0,0, wxALPHA_OPAQUE), 1, wxSOLID );
	static wxBrush brush_white( wxColour(255,255,255, wxALPHA_OPAQUE) );
	static wxBrush brush_black( wxColour(0,0,0, wxALPHA_OPAQUE) );

	int radius = MIN( m_Rect.width, m_Rect.height );
	radius /= 2;
	dc.SetPen( pen_white );
	dc.SetBrush( brush_white );
	dc.DrawCircle( m_Rect.x + (m_Rect.width / 2), m_Rect.y + (m_Rect.height / 2), radius + 1 );

	wxPoint *points = NULL;

	switch( m_Placement )
	{
	case PLACE_TOP:
		points = wxConnexInArrows[IsInput()?PLACE_TOP:PLACE_BOTTOM];
		break;
	case PLACE_BOTTOM:
		points = wxConnexInArrows[IsInput()?PLACE_BOTTOM:PLACE_TOP];
		break;
	case PLACE_LEFT:
		points = wxConnexInArrows[IsInput()?PLACE_LEFT:PLACE_RIGHT];
		break;
	case PLACE_RIGHT:
		points = wxConnexInArrows[IsInput()?PLACE_RIGHT:PLACE_LEFT];
		break;
	}

	if( points )
	{
		dc.SetBrush( brush_black );
		dc.SetPen( pen_black );
		dc.DrawPolygon( 3, points, m_Rect.x, m_Rect.y );
	}
}
