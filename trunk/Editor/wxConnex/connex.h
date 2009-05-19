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

#ifndef _CONNEX_H
#define _CONNEX_H

#include <list>
#include "wx/gdicmn.h"
#include "wx/dcclient.h"

#ifndef MIN
#define MIN(__lx, __rx) (__lx < __rx ? __lx : __rx)
#endif

class wxConnexNode;

extern wxPoint wxConnexInArrows[4][3];
extern wxPoint wxConnexSplineDelta[4];

using namespace std;

class wxConnexPortClass
{
public:
	const char*	m_Name;
	size_t		m_RunTimeID;

	wxConnexPortClass( const char* name ) { m_Name = name; m_RunTimeID = (size_t)name; }
};

#define DECL_PORT_CLASS() \
	public: \
	static wxConnexPortClass ms_PortClass; \
	virtual wxConnexPortClass* GetPortClass() const { return &ms_PortClass; }

#define DEF_PORT_CLASS_NO_BASE( _class ) \
	wxConnexPortClass _class::ms_PortClass( #_class );

#define DEF_PORT_CLASS( _class, _base_class ) \
	wxConnexPortClass _class::ms_PortClass( #_class );

#define INVALID_PORT_ID ((unsigned int)-1)

class wxConnexPortBase;

typedef std::list<wxConnexPortBase*>	PortList;
typedef PortList::iterator				PortListIterator;

class wxConnexPortBase
{
public:
	static unsigned int ms_NextPortID;

	enum PORT_DIR
	{
		DIR_INPUT,
		DIR_OUTPUT
	};

	enum PLACEMENT
	{
		PLACE_TOP,
		PLACE_BOTTOM,
		PLACE_LEFT,
		PLACE_RIGHT
	};

protected:
	unsigned int	m_ID;
	PORT_DIR		m_Direction;

public:
	wxRect					m_Rect;
	PLACEMENT				m_Placement;
	PortList				m_Connections;
	wxConnexNode			*m_Parent;

public:
	wxConnexPortBase();
	wxConnexPortBase( PORT_DIR dir, PLACEMENT place = PLACE_LEFT, unsigned int id = INVALID_PORT_ID );
	unsigned int GetID() const			{ return m_ID; }
	bool IsInput() const				{ return m_Direction == DIR_INPUT; }
	void SetDirection( PORT_DIR dir )	{ m_Direction = dir; }

	int GetNbrConnections() const { return (int)m_Connections.size(); }
	bool HasConnections() const { return !m_Connections.empty(); }
	bool IsConnected( wxConnexPortBase *port );
	void RemoveConnection( wxConnexPortBase *port );
	void RemoveAllConnections();

	//////////////////////////////////////////////////////////////////////////
	//tell your connections your data has changed;
	void RefreshConnections();

	//////////////////////////////////////////////////////////////////////////
	virtual void OnInputUpdate( wxConnexPortBase *port )				{ }
	virtual bool CanTakeAsInput( const wxConnexPortClass &port_class )	{ return false; }
	virtual void Paint( wxPaintDC &dc, wxPoint normal );

	//////////////////////////////////////////////////////////////////////////
	bool CanTakeAsInput( const wxConnexPortBase &port )			{ return CanTakeAsInput( *port.GetPortClass() ); }

	DECL_PORT_CLASS();
};

#endif
