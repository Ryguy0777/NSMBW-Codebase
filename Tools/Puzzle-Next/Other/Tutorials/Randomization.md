# Tile Randomization

This allows for tiles to randomly select another tile (from a pre-defined list) to display in-game, allowing for variation and
less repetition in your tilesets.

Before you can make randomization, make sure you actually have alternative tiles to be picked from. Ideally you should aim for
having 6 total types of floor, filling ground, and ceiling tiles, as well as 4 wall tiles, like Nintendo's tilesets.

## RandTile Entries
Entries can be created by going into the RandTiles table and clicking **Add Entry**. This will create a new row in the table,
with the following options:

| Option | Description |
| ------------ | --- |
| Tile Number  | The tile number that will be randomized in-game. |
| Normal Type  | Controls how the tile randomization will be applied (in what direction). |
| Special Type | Similar to the above, but for 2x2 objects, such as the large Mini Mega tileset. |
| Random Tile Values | List of the possible tiles that can be randomly selected. |

The tile values should be formatted with a `,` after each value (besides the last value). An example would be `0x5B, 0x5C, 0x5D, 0x5E`.

The type values control how the tiles will be randomized in-game. These are the possible string values:

| Normal Type | Behavior |
| ----------- | --- |
| None | No randomization in-game |
| Horizontal | The tile will only be randomized horizontally (Ex. ground, ceilings) |
| Vertical | The tile will only be randomized vertically (Ex. walls) |
| Both | The tile will be randomized in both directions (Ex. ground fill) |

| Special Type | Behavior |
| ------------ | --- |
| None | No randomization in-game |
| VDoubleTop | Defines the top row of a 2x2 object that will be randomized |
| VDoubleBottom | Defines the bottom row of a 2x2 object that will be randomized |
| HDoubleLeft | Defines the left column of a 2x2 object that will be randomized |
| HDoubleRight | Defines the right column of a 2x2 object that will be randomized |

## Importing/Exporting
To save your randomization, simply save the tileset itself. The RandTile data is saved in the `BG_rand` directory.

If you plan on re-using the same randomization data across multiple tilesets (highly recommended as many tilesets are organized similarly),
you can use the Import/Export buttons at the bottom of the tab to do so. It doesn't matter which format you export/import as, however
JSON allows for easy editing outside of Puzzle, if needed.