# Einführung #

das gui (graphical user interface)-system kümmert sich um die grafische in-game-benutzerschnittstelle, also vor allem um fenster und anzeigen.
das system ist sehr simpel gehalten: es kennt nur fenster und unter-fenster (letztere allerdings nur um etwas mehr geschwindigkeit zu erhalten, es ginge auch nur mit fenstern). diese fenster können erstellt, zerstört, versteckt und verschoben werden. zusätzlich ist es mit callback-funktionen möglich auf allerhand benutzereingaben zu reagieren (z.b. tastendrücke, mausklicks, mausbewegungen etc). einem fenster kann ein ogre-material zugewiesen werden, welches es dann anzeigt. fenster werden über einen eindeutigen handle identifiziert. bevor ein fenster angezeigt werden kann muss es "gebacken" werden. alle fenster und unter-fenster müssen erstellt sein, bevor es gebacken wird, ansonsten werden sie nicht angezeigt werden.
das system kümmert sich um genau ein fenster selbst: die maus. alle anderen fenster sind ausschließlich script-code. die fenster werden im master-script initialisiert.

um ein fenster zu erstellen und anzuzeigen ist folgender code erforderlich:

```
handle=gui_create_window(0.25,0.25,0.5, 0.5);
gui_bake_window(handle);
gui_set_window_material(handle, "yourmaterial");
```

# gui-funktionen #

koordinaten liegen im intervall [`0:1`], die gilt auch für sub-fenster, für die jedoch das übergeordnete fenster die bezugsbasis darstellt

## fenster ##

`gui_create_window(float x, float y, float w, float h[, float fUScale, float fVScale])`

  * `x,y` X und Y-Koordinate des fenster
  * `w,h` breite und höhe des fensters
  * `[fUScale, fVScale]` skalierung der textur
  * `return` window handle


`gui_create_subwindow(int iParentHandle, float x, float y, float w, float h[, float fUScale, float fVScale])`
  * `iParentHandle` handle des parent-fensters
  * `x,y` X und Y-Koordinate des fenster
  * `w,h` breite und höhe des fensters
  * `[fUScale, fVScale]` skalierung der textur
  * `return` sub window handle

`gui_bake_window(int handle)`
  * `handle` window-handle des zu backenden fensters

`gui_delete_window(int handle)`
  * `handle` window-handle des zu löschenden fensters

`gui_set_window_material(string material)`
  * `material` ogre material string

`gui_set_visible(int handle, bool visible)`
  * `handle` window handle des zu zeigenden fensters
  * `visible` true: fenster sichbar; false: nicht.

`gui_set_foreground_window(int handle)`
  * `handle` window-handle des in den vordergrund zu schiebenden fensters


`gui_set_focus(int handle)`
  * `handle` window-handle des zu fokussierenden fensters. das fenster mit dem fokus erhält die tastatur-eingaben

`gui_move_window(int handle, float x, float y)`
  * `handle` window-handle des zu verschiebenden fensters
  * `x,y` X und Y-Koordinate des fensters

`gui_get_window_pos(int handle)`
  * `handle` handle des fensters, dessen position abgefragt wird
  * `return` x,y-koordinate des fensters

`gui_set_cursor_pos(float x, float y)`
  * `x,y` X und Y-Koordinate auf die die maus zu setzen ist

`gui_get_screen_coords(int handle)`
  * `return` x,y-koordinate des fensters in bildschirmkoordinaten
  * `return` breite und höhe des fensters in bildschirmkoordinaten

`gui_get_cursor()`
  * `return` handle des cursor-fensters

`gui_set_cursor(int handle)`
  * `handle` handle des fensters, das als cursor gesetzt werden soll


## fonts ##

`gui_create_font_texture(string spacing_file, string text, int maxwidth, int maxheight)`
  * `spacing_file` die spacing-datei des fonts
  * `text` selbstredend der text, der auf der textur stehen soll
  * `maxwidth, maxheight` maximale höhe und breite der textur
  * `return` texture\_name: name der erstellten textur
  * `return` actual\_width: tatsächliche breite der textur
  * `return` actual\_height: tatsächliche höhe der textur


`gui_delete_texture(string texture)`
  * `texture` textur, die gelöscht werden soll (geht mit allen ogre-texturen)

`gui_create_font_material(string spacing_file, string texture_name, string base_material_name, string pass_name, string target_texture_name, int maxwidth, int maxheight)`
  * `spacing_file` die spacing-datei des fonts
  * `texture_name` font-textur-name
  * `base_material_name` name des materials, aus dem das font-material erzeugt wird
  * `pass_name` pass im basis-material, der verändert werden soll
  * `target_texture_name` texturname im basis-material im basis-pass, der verändert werden soll (in die angegebene textur)
  * `maxwidth, maxheight` maximale höhe und breite der textur
  * `return` material name

`gui_change_font_material(string spacing_file, string material_name, string base_material_name, string pass_name, string target_texture_name, int maxwidth, int maxheight)`
  * `spacing_file` die spacing-datei des fonts
  * `material_name` material, das geändert werden soll
  * `texture_name` font-textur-name
  * `pass_name` pass im basis-material, der verändert werden soll
  * `target_texture_name` texturname im basis-material im basis-pass, der verändert werden soll (in die angegebene textur)
  * `maxwidth, maxheight` maximale höhe und breite der textur

## mouse events ##

`gui_set_on_click_callback`

`gui_set_mouse_down_callback`

`gui_set_mouse_up_callback`

`gui_set_mouse_move_callback`

`gui_set_mouse_hover_in_callback`

`gui_set_mouse_hover_out_callback`


## keyboard events ##

`gui_set_on_char_callback`

`gui_set_key_down_callback`

`gui_set_key_up_callback`


## input states ##

`input_get_key_down`

`input_get_mouse_buttons`