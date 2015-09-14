# SchemeSystem #
  * Reiter im Edit
  * GOComponent GOCScheme
  * Schemes werden über GOCScheme den Objekten zugeordnet
  * Parameter-passing der Nodes über Message-Map des Message-Systems
  * Erste Nodes des Schemas werden sofort beim init des Objekts "live" geschaltet (sollten meist listener oder init-zeugs sein)
  * Auführungspfad wird bei State-Change durch System traversiert
  * Erste Nodes des Schemas werden nach Abschluss nicht gelöscht (falls blockende Nodes), da sie immer vorhanden sein müssen und sonst dauernd respawnt würden
  * Alle anderen nach Abschluss deleted
  * Bedingte Verzweigungen (z.B. listener für ausgewählte Dialog-Option)?
  * System soll Thread-sicher reagieren, in Bezug auf die finished()-calls der blockenden nodes


## Nodes ##
  * GUI und funktionale Definition im Script
  * optional als Plugin in C-Code (für zeitkritisches zeugs)


## Node-Typen ##
  * Blocking-Node (wartet auf Messages etc. benötigt callback)
  * Abgeleitet von SchemeSystem::Node
  * ruft finished() auf, delete bei beendigung des ausführungspfades, bzw bei erreichen einer blockenden Node (außer erste Nodes des Schemas, s.o.)
  * Non-Blocking-Node (sendet massages, greift aufs api zu etc.
  * prinzipiell nur eine static-funktion
  * wird für gewöhnlich im finished() einer Blocking-Node ausgeführt



## Schemes ##
  * Erstellt mit Prozesseditor
  * Eigene Savefiles?
  * Bilden Definition des Schemas "offline" ab
  * "Live"-Nodes sind von anderem Typ als diese "Schatten"-Nodes
  * Schatten-Nodes:
```
	static const bool bNonBlocking;
	void Run(Args);//bei Non-Blocking: Argumente der vorherigen
	SchemeSystem::Node  * create(Args);//bei Blocking: übergibt Node-Einstellungen an die "Live"-Node, Args von der vorherigen Node
	private Node_Data;
```

## GOCScheme ##
  * hält Schemes pro Objekt
  * Schnittstelle bereitstellen für Objektnachrichten