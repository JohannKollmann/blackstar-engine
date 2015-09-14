# AI #

## Inputs ##

Sinne:

  * Hören
  * Sehen
  * riechen? (vllt cool wenn minen mit grubengas gefüllt sind und explodieren können oder einen betäuben)
  * fühlen

## Outputs ##

  * gehen/rennen (mit wegfindung)
  * sprache
  * kopf-ausrichtung (maximale sinnesempfindlichkeit)
  * symbolische handlungen (z.b. "hände hoch", wirkt wie sprachnachricht)
  * kampf-animationen (mit kampfsystem zusammen)
  * spezial-animationen (für quests etc.)

## Allgemeine Eigenschaften des Systems ##

  * reize sind räumlich und zeitlich (jedenfalls größtenteils) begrenzt, sozusagen "bubbles", lassen sich mit einfachen physik-shapes abbilden
  * wenn ein reiz ausgelöst wird:
    * (1) alle receiver in maximaler reichweite des reizes ermitteln, diese testen, ob sie den reiz empfangen können. dieses im zeitlichen abstand widerholen
    * (2) eine bubble erzeugen und im raum "schweben" lassen. npcs checken regelmäßig ihre umgebung.
  * (denke beide verfahren sind in etwa gleichwertig, aber ich würde verfahren (1) favorisieren)
  * es sollte einen mechanismus geben, der es npcs ermöglicht, einen reiz als "akzeptiert" zu markieren, sodass er keine ressouren auf dem npc verschwendet

## Umsetzung ##

  * sobald eine handlung einen reiz auslösen soll, wird dem ai-system ort, ausdehnung, dauer, auslöser, id gemeldet. das system legt eine bubble an.
  * goc-sense führt regelmäßige sweeps durch, unter berücksichtigung der aktuellen kopf-ausrichtung. die größe der sweep-shapes hängen von den eigenschaften des npcs ab (ein adler wird eine größere visual-shape haben als ein maulwurf).
  * bubbles werden für den jeweiligen sense erstellt und erhalten eine id für den event.
  * npc-scripte bekommen nachrichten von goc-sense, können jeweiligen event "akzeptieren" und damit in zukunft unterdrücken. events werden für ein npc-script abonniert und goc-sense sucht nur danach.

## Scripting-Interface ##
```
registerSenseEvent(sense_id, event_id, callback);
function callback(sense_id, event_id, data)
```

vllt wäre es auch möglich, direkt die message-system schnittstellen zu verwenden, in lua haben wir damit aber noch nicht wirklich gearbeitet, da muss sowieso noch was designed werden

hier wäre es sehr sinnvoll, den lua map-datentyp anzuwenden, da man die datenfelder dann nativ in lua iterieren kann
für den datenaustausch wird die message system datamap verwendet
geeignete funktionen dafür sind bereit zu stellen, am besten direkt als lua-map übergeben sonst z.b.

```
data_id=createDataMap()
setDataMapField(data_id, field_id, data)
getDataMapField(data_id, field_id)
```

momentan wird ohne feldnamen einfach sequentiell übergeben (ScriptMessageListener), finde ich aber nicht sonderlich gut zu lesen und fehlerträchtig

posten eines events:
```
postSenseEvent(sense_id, event_id, data_id)
```

### Erstellen von bubbles ###

Bubbles sind PhysX-Objekte mit einem speziellen Flag. Das Script erstellt dazu ganz regulär Objekte mit GOCRigidBody Komponente oder verwendet existierende.

### Use-case eventdata ###

ein diebstahl wird beobachtet
```
SENSE_VISUAL, EVENT_THEFT:
ID_ORIGIN //id des verursachers des events (ein npc)
ID_OBJECT //id des gestohlenen objektes

//folgende felder nur beim posten, nicht beim receive
ID_BUBBLE /* bubble, hier könnte z.b. eine bubble eingetragen werden,
die an die hände des hand-bones attached wurde.
so wären z.b. diebstähle möglich, solange man sich
mit dem körper vor dem blick der wachen entziehen kann.
ansonsten wäre natürlich auch eine große bubble um den npc herum möglich*/
ID_DURATION /*zeit in sekunden als float, wie lange das event aktiv sein soll
(z.b. 2.0 sekunden, bis die verräterische animation abgespielt wurde
und keiner es mehr erkennen kann)*/
```

ein noch coolerer ansatz wäre, ein generisches "pickup"-event zu machen, und die beobachtetenden npcs müssen beurteilen ob es ein diebstahl ist oder nicht, finde es iwie doof, wenn auf magische art und weise alle wissen welcher gegenstand rechtmäßig zu nehmen ist und welcher nicht. passanten, die das event beobachten, könnten den vermeintlich beklauten ansprechen und klären ob es ein diebstahl war und dann erst alarmieren, würde die sache deutlich realistischer machen.

ein anwohner ruft um hilfe, weil er einen dieb bei frischer tat ertappt hat
```
SENSE_AUDITIVE, EVENT_ALERT
ID_ORIGIN //der anwohner
ID_OBJECT //der stehlende npc
ID_ALERT_TYPE //ID_THEFT
ID_BUBBLE //id der ruf-bubble des anwohners
ID_DURATION //5 sek, bis der hilferuf vorbei ist
```
eine wache reagiert auf den hilferuf
```
SENSE_AUDITIVE, EVENT_ALERT
ID_ORIGIN //die wache
ID_OBJECT //der stehlende npc
ID_ALERT_TYPE //ID_THEFT
ID_BUBBLE //id der ruf-bubble der wache
ID_DURATION //5 sek, bis der hilferuf vorbei ist
```

hier wird der anwohner den ruf der wache mitkriegen und kann seinen alert-state verlassen, ansonsten müsste er in einen panik-modus verfallen und überall um hilfe rufen, was andere anwohner auch in panik versetzt etc. sieht bestimmt lustig aus!
irgendwann wird dann eine wache reagieren und der mob beruhigt sich wieder.

weil schon solch einfach dinge derart viele statische daten enthalten müssen, wäre ich dafür die konkreten prozesse grafisch zu erstellen, denn als script wird man sich in der masse nicht mehr gut zurecht finden können.