# Profile Overrides

This feature is more complex, it allows for you to make tiles that are overriden with actors in-game. Some examples of this
from the retail game are the rails from some Pa3 tilesets.

## Entries
Entries can be created by going into the Profile Overrides table and clicking **Add Entry**. This will create a new row in
the table, with the following customization options:

| Option | Description |
| ----------- | --- |
| Tile Number | The tile number that will be overriden in-game. |
| Profile ID  | The ID of the profile (actor) that will be spawned instead of the tile.  |
| Offset X / Y / Z | These values control the position offset of the actor, relative to the tile position. |
| Scale X / Y | Size that the actor takes up. This is used when creating/deleting the actor to ensure it doesn't occur while still on/off-screen. |
| Param | The parameters ("spritedata") passed to the actor. Please note, nybble 10 will OR'd by the layer that this tile is on. |
| Rail Color | [Optional] HEX color applied to rails. Should use the format `0xAABBCC`. |
| Rail SRT X / Y | [Optional] Scale of the material applied to rails. |

Keep in mind that this is meant to be used for rails, however it can be used for other actors too.

## Importing/Exporting
To save your overrides, simply save the tileset itself. The data is saved in the `BG_prof` directory.

If you plan on re-using the same data across multiple tilesets, you can use the Import/Export buttons
at the bottom of the tab to do so. It doesn't matter which format you export/import as, however JSON
allows for easy editing outside of Puzzle, if needed.