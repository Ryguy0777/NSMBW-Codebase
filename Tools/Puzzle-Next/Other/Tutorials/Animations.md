# Animated Tiles

This allows for you to have tiles that play animations instead of being static, similar to ? Blocks or Brick Blocks.

## Framesheets
The first thing you need is a framesheet. This is a texture of all of the different frames of your tiles, stacked on top of each other.
Before you can import your framesheet, however, these textures need to be *clamped*. Clamping is where each edge of a tile is copied out
by 4 pixels to avoid graphical issues in-game.

You can either manually clamp your tileset image, or you can allow Puzzle to do it. Go to `Framesheets > Create clamped framesheet` to open
a dialog where you can select your image, and then save the clamped version of that framesheet.

As a general reference, your clamped framesheet should be 32px wide, and 32px * the number of tiles tall. Puzzle does not allow you to
import framesheets that do not fit the proper dimensions.

### Importing
To import your framesheet, go to the Framesheets tab, and click the `Add` button. Once imported, we need to rename it so it can be recognized
in-game. Framesheets use the naming scheme `anim_XX`, where `XX` is the tile number it applies to. This can be found by hovering over a tile,
and combining its row and column. For example, if a tile is on Row 0x5, Column 0xB, its tile number is `0x5B` (or `5B`).

### Exporting
To export a framesheet, double-click on it in the tab, and you will be prompted to save it. To export *all* framesheets, see the first two
options in the `Framesheets` dropdown.

## AnimTile Entries
Once you have a framesheet entered, we need to create an AnimTile entry for it. This will allow the game to animate it, and tell it *how*
it should be animated. Go to the AnimTiles tab, and click on `Add Entry` to create a new entry. This will add a row to the table with
three options, which are explained below:

| Option | Description |
| ------------ | --- |
| Tile Number  | The tile number this will apply to. Also controls which framesheet texture will be loaded. |
| Is Reversed  | Controls if the animation will be played in reverse. |
| Frame Delays | Delay (in frames) before each frame of the animation. This also controls the number of frames in the animation. |

The frame delays should be formatted with a `,` after each value (besides the last value). An example would be `0x4, 0x8, 0x4, 0x8`.

## Importing/Exporting
To save your animation, simply save the tileset itself. The animation data is saved in the `BG_anim` directory.

If you plan on re-using the same animation data across multiple tilesets, you can use the Import/Export buttons at the bottom
of the tab to do so. It doesn't matter which format you export/import as, however JSON allows for easy editing outside of Puzzle, if needed.