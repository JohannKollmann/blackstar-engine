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

#ifndef _CONNEX_NODE_H
#define _CONNEX_NODE_H

#include "connex.h"
#include "wx/window.h"

class wxConnexSpace;
class wxConnexPortBase;

class wxConnexNode : public wxWindow
{
public:
	wxConnexSpace		*m_Space;
	wxSize				m_NonScaleSize;
	wxPoint				m_ClickPoint;

	PortList			m_Ports;
	wxConnexPortBase	*m_FocusPort;

public:
	wxConnexNode();
	wxConnexNode(	wxConnexSpace *parent, wxWindowID id = wxID_ANY,
					const wxPoint& pos = wxDefaultPosition,
					const wxSize& size = wxSize(50, 50), long style = wxNO_BORDER | wxCAPTION,
					const wxValidator& validator = wxDefaultValidator,
					const wxString& name = wxPanelNameStr );

	void AddCXPort( wxConnexPortBase *port );
	wxConnexPortBase* GetCXPort( const wxPoint &client_pos );

	//when an input port data/value has changed this will get called
	//if port == NULL you should refresh all your ports
	virtual void OnPortDataUpdate( wxConnexPortBase* port ) {}

	//////////////////////////////////////////////////////////////////////////
	void OnPaint( wxPaintEvent& event );
	void OnMouseEvent( wxMouseEvent &event );

	//////////////////////////////////////////////////////////////////////////
	DECLARE_CLASS( wxConnexNode )
	DECLARE_EVENT_TABLE()
};

#endif
