
#ifndef __CustomSizer_H__
#define __CustomSizer_H__

#include "wx/wx.h"
#include "Ogre.h"		//nur f�r Log!

/*
Eigene kleine Layoutklasse, da die Originalsizer vOn wxWidgets
mysteri�serweise nicht bei einem Window/Panel funktionieren.
Die Klasse arbeitet Tabellenbasiert: Add f�gt ein weiteres Widget der aktuellen Spalte hinzu,
newline erstellt eine neue Spalte.
*/

class wxCustomSizer
{
	private:
		wxWindow *mWindow;
		float mCurrentLinePosition;

	public:
		wxCustomSizer(wxWindow *window);
		~wxCustomSizer() { };

		void Reset();

		/*
		Erstellt eine neue Spalte. distance_in_percent gibt an,
		wie viel Prozent des Fensters Platz zwischen den Spalten sein soll.
		*/
		void newLine(float distance_in_percent);

		/*
		F�gt ein Widget der aktuellen Spalte hinzu. Percent gib an, wo das widget erstellt werden soll.
		yCenter gibt an, ob die Mitte des Widgets auf der aktuellen Spalte liegt.
		*/
		void Add(wxWindow *widget, float percent, bool xCenter = false, bool yCenter = true);

		/*
		Resized das Widget zus�tzlich so, dass es genau den Raum zwischen percentFrom und To ausf�llt.
		*/
		void Add(wxWindow *widget, float percentFrom, float percentTo, bool yCenter = true);
};
#endif