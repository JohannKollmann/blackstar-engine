## Einleitung ##
Das Nachichtensystem regelt die Kommunikation zwischen den verschiedenen Komponenten der Engine. Der größte Vorteil ist, dass man mit dem Messagesystem Nachichten senden kann, ohne den bzw. die Empfänger zu kennen. In der main Loop zum Beispiel wird in jedem Schleifendurchlauf eine UPDATE\_PER\_FRAME Message gesendet mit der Zeit seit dem letzten Frame als Parameter. Instanzen anderer Klassen könnten nun dem message System mitteilen, sie würden gerne UPDATE\_PER\_FRAME Nachichten empfangen. Ein weiteres simples Anwendungsbeispiel ist die Inputverwaltung.


## Details ##
1. Grundidee

Jede Klasse, deren Objekte Messages empfangen können sollen, muss von der abstrakten Basisklasse MessageListener erben und über eine Methode void ReceiveMessage(Msg &msg) verfügen. Gesendet werden Messages über die Methode MessageSystem::SendMessage(Msg &msg), die über die Singletoninstanz von MessageSystem überall verfügbar ist. Messages gehören grundsätzlich einer Newsgroup an. Diese Newsgroup dient zum einen der Erkennung der Messages, zum anderen können MessageReceiver sich in MessageSystem als Listener für bestimmte Newsgroups eintragen. Ein Sender muss den Empfänger der Nachicht also nicht zwingend kennen. Optional ist es auch möglich, die Message nur an einen ganz bestimmten Empfänger unabhängig von Newsgroups zu senden.

2. Die Nachichtenklasse Msg

In dieser Struktur wird alles gespeichert - der Newsgroupname (Default: "COMMON"), der optionale Empfänger-Zeiger (Default: NULL) und die eigentlichen Daten. Diese werden mithilfe einer Instanz der Klasse DataMap gespeichert, die intern eine Map aus Ogre::String und void Zeigern verwaltet. Es stehen komfortable selbsterklärende Methoden bereit, um die Map zu beschreiben oder auszulesen.


## Nachichten ##

**Allgemeine Form**:
  * [Newsgroupname](Newsgroupname.md)
    1. Parametername1 (Typ)
    1. Parametername2 (Typ)
    1. ...

**Allgemein**:

  * UPDATE\_PER\_FRAME
    1. TIME (float)
  * INGAME\_CONSOLE
    1. COMMAND (Ogre::String)
    1. ...


**Input**:

  * KEY\_DOWN
    1. KEY\_ID (OIS::KeyCode)
  * KEY\_UP
    1. KEY\_ID (OIS::KeyCode)
  * MOUSE\_DOWN
    1. MOUSE\_ID (OIS::MouseButtonID)
  * MOUSE\_UP
    1. MOUSE\_ID (OIS::MouseButtonID)
  * MOUSE\_MOVE
    1. ROT\_X\_ABS (int)
    1. ROT\_X\_REL (int)
    1. ROT\_Y\_ABS (int)
    1. ROT\_Y\_REL (int)