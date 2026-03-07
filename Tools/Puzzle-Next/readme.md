# Puzzle Next
This is the Puzzle NSMBW Tileset Editor, licensed under GNU GPL v2 (see `LICENSE`).

This version is designed to load and save various formats from within the tileset archive. These include:
- Animated tiles
- Tile randomization
- Flower/Grass overrides
- Flower/Grass BRRES files
- Profile overrides

These formats are stored within the tileset to allow for maximum portability, they can be dropped into
any PropelParts project and will work without needing to edit global AnimTiles/RandTiles binaries like
in Newer and NSMBWer+ based projects.

For more information on using these new formats, see the help articles in `/Other/Tutorials/`, or the `Tutorials`
dropdown within Puzzle.

In order to use this version, you need to add Tileset Tools. Puzzle makes use of these to encode/decode the
various formats. These can be found [here][tilesettools]. Move the following files into Puzzle's `tools` folder:
* `anim.py`
* `grass.py`
* `profoverride.py`
* `rand.py`

Once done, everything is setup to run Puzzle Next.

# History
- The Puzzle NSMBW Tileset Editor was created by Tempus.
	* The original program can be found at https://github.com/Tempus/Puzzle
- The Puzzle Updated fork by RoadrunnerWMC can be found at: https://github.com/NSMBW-Community/Puzzle-Updated
	* Python 3, Qt-for-Python/PyQt5 porting
	* Black border fix
- Puzzle Next can be found at: https://github.com/NSMBW-Community/Puzzle-Next
	* Adds tile animation and randomization support
	* Additional tile object settings (ported from Abood's fork)
	* Dark Mode

# Requirements
### Running from Source
- Python 3: https://www.python.org
- PyQt5: `py -3 -m pip install PyQt5`
- Execute `puzzle.py`

### Build
Simply execute the binary included in your download.

[tilesettools]: https://github.com/Ryguy0777/PropelParts/tree/main/Tools/tilesettools