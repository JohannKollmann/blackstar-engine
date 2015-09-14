# Einleitung #

Alles, was im dreidimensionalem Raum irgendwo platziert ist, ist ein Objekt - eine Kiste, ein Npc, ein Licht, ein Trigger usw.
Die entsprechende Klasse heißt GameObject.


# Das Komponentensystem #

Die Klasse GameObject speichert an Objekteigenschaften lediglich die globale Transformation eines Objekts. Es ist außerdem nicht vorgesehen, dass andere Klassen von GameObject erben. Ein Objekt besteht stattdessen aus einer Kollektion von Komponenten. Komponentenklassen erben grundsätzlich von GOComponent (GO = Game Object). Eine Komponente implementiert eine bestimmte Funktionalität, die dem Objekt hinzugefügt werden kann.
Ein gutes Beispiel ist Feuer: Hier würde man ein Objekt erstellen mit den Komponenten 'Partikeleffekt', 'Sound' und 'Licht'. Auch Physik-Objekte werden auf diese Weise implementiert: Eine Kiste besteht zum einen aus der grafischen Repräsentation, einem Kisten-Mesh, zum anderen aus einem Kollisionsmodell, welches von der Physikengine berechnet wird.


### Einschränkungen ###

Einem Objekt können theoretisch beliebig viele Komponenten hinzugefügt werden, allerdings darf ein Objekt jeweils nur ein Element einer Komponentenfamilie besitzen. Diese Einschränkung ist wichtig, um nach außen eine einheitliche Schnittstelle anzubieten, mit der auf Komponentenfamilien zugegriffen werden kann.

### Das lokale Nachichtensystem ###

Komponenten sollten prinzipiell unabhängig voneinander sein, allerdings ist es manchmal unerlässlich, dass sie miteinander kommunizieren. Mit dem lokalen Nachichtensystem existiert dafür eine komfortable Lösung. Wie beim globalen Nachichtensystem können Nachichten gesendet werden, ohne den Empfänger zu kennen - das lokale Nachichtensystem eines Objekts verzichtet aber auf Newsgroup-Registrierungen. Jede Nachicht wird an jede Komponente gesendet, die GOComponent::ReceiveObjectMessage(Msg &msg) implementiert.

### Beispiele ###

```
//Pfx-Effekt mit Licht
GameObject *object = new GameObject();
object->AddComponent(GOComponentPtr(new GOCPfxRenderable()))
object->AddComponent(GOComponentPtr(new GOCLocalLightRenderable()))
((GOCPfxRenderable*object->GetComponent("ParticleSystem"))->SetParameters(my_data_map); 	//Beispielzugriff über Familienname
object->GetComponent<GOCPfxRenderable>()->SetParameters(my_data_map); 	//Beispielzugriff über template
```

```
//Player
GameObject* player = new GameObject();
player->AddComponent(GOComponentPtr(new GOCPlayerInput()));
player->AddComponent(GOComponentPtr(new GOCCameraController()));
player->AddComponent(GOComponentPtr(new GOCCharacterController()));
player->AddComponent(GOComponentPtr(new GOCAnimatedCharacter()));
player->SetGlobalPosition(Ogre::Vector3(0,10,0));
```