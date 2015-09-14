### Einleitung ###
Mit dem Editor lassen sich Objekte platzieren - Lichter, Meshes, Npcs, Waypoints usw.
Damit man nicht für jede neue Komponentenklasse, die im Editor benutzbar sein soll, den Gui-Code verändern muss, wurde das Editor Interface geschaffen. Klassen, die das Editor Interface bereitstellen wollen, müssen die Klasse EditorInterface implementieren. Komponenten müssen die Klasse GOCEditorInterface implementieren.

### Funktionsweise ###
Klassen, die das Editor Interface implementieren, müssen gewisse Methoden bereitstellen, die vom Editor benutzt werden:

`void SetParameters(DataMap *parameters)`
Diese setzt eine Menge von Parametern, die in einer Datamap übergeben werden. Eine Datamap speichert eine Map von Parametern, die vom Typ bool, int, float String, Vector3, Quaternion oder ColourValue sein können. Jeder Parameter kann über einen String-Key angesprochen werden.
Der Editor kann die Datamap in einer übersichtlichen PropertyGrid anzeigen.

`void GetParameters(DataMap *parameters)`
In dieser Methode muss das Objekt seine aktuellen Zustandsparameter in die übergebene Datamap schreiben.

`void GetDefaultParameters(DataMap *parameters)`
In dieser Methode muss das Objekt seine Standardparameter in die übergebene Datamap schreiben.

`GOComponent* GetGOComponent()` Liefert die mit dem Editor Interface assoziierte Komponente zurück, in der Regel this.

Des weiteren muss das Cloneable Interface implementiert werden, bzw. die Methode New, die eine neue Instan der eigenen Klasse erstellt.

### Registrierung ###
Damit die Klasse, die das Editor Interface bereitstellt, auch wirklich im Editor sichtbar ist, muss das Interface registriert werden.
```
void SceneManager::Init()
{
...
   RegisterGOCPrototype("A", GOCEditorInterfacePtr(new GOCMeshRenderable()));
...
}
```
"A" gibt die interne Gruppe an, wobei diese alphabetisch sortiert werden. Dies hat Einfluss auf die Gui-Darstellung im Editor.