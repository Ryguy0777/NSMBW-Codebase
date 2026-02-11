# Flower and Grass Overrides

This allows for you to override grass/flower tiles with animated, interactive grass and flowers, like with the retail grassland and
forest tilesets. Please note that only Pa1 tilesets can have grass or flowers.

## Entries
The first thing to do is to create tile entries. This will allow the game to override these tiles with flowers/grass.
Go to the Flower/Grass Overrides tab, and click on `Add Entry` to create a new entry. This will add a row to the table with some options,
which are explained below:

| Option | Description |
| ------------ | --- |
| Tile Number  | The tile number this will apply to. This is the row and column of the tile. |
| Flower Type  | The type of flower on this tile (if any). See below for the different types. |
| Grass Type   | The type of grass on this tile (if any). See below for the different types. |

The flower/grass types can be stacked, as seen in retail tilesets (flowers on top of grass). Values are as follows:

| Flower Type | Description |
| --- | --- |
| 0 | No flower |
| 1 | Yellow flower |
| 2 | Pink flower |
| 3 | Blue flower |

| Grass Type | Description |
| --- | --- |
| 0 | No grass |
| 1 | Left-most grass |
| 2 | Middle-left grass |
| 3 | Center grass (used for flower + grass tiles) |
| 4 | Middle-right grass |
| 5 | Right-most grass |

## Importing/Exporting Entry Data
To save the flower/grass overrides, simply save the tileset. The data is saved in the `BG_grass` directory.

If you plan on re-using the same data across multiple tilesets, you can use the Import/Export buttons at the bottom
of the tab to do so. It doesn't matter which format you export/import as, however JSON allows for easy editing outside of Puzzle, if needed.

## Resource Management
In addition to controlling what tiles use grass/flowers, you can also change how they look. The Flower File/Grass File options
control what resources are loaded. You can use the default grassland grass/flowers, forest ones, or custom ones.

Custom resources are loaded from the tileset archive. If using the custom option, you *must* import BRRES files into the tileset.
The Import options will do this for you. You can also export the BRRES files if needed.

The filename you import doesn't matter, as it will automatically save under the proper filenames. For reference, however,
grass uses `obj_kusa.brres`, and flowers use `obj_hana.brres`. These can be obtained from the retail `Object` folder, and
modified as you'd like.

Like with the override data, you can save these files by saving the tileset archive.
