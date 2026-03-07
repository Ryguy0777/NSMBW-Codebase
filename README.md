# PropelParts
This is a custom code platform for New Super Mario Bros. Wii that adds new features to the game, intended to serve as a basis for mods of the game.
Currently still in development, so don't expect everything to be finished. You can check `FEATURELIST.md` to see a list of features implemented, planned, and unfinished.

## Usage
You are expected to be familiar with NSMBW modding and how to use Riivolution and Reggie! Next.

A build of PropelParts is provided in the Riivolution folder. 

Extract this folder to the root of your SD card if using a real Wii, and `[Dolphin User Folder]/Load/Riivolution/` if using Dolphin Emulator. You can open the User Folder from Dolphin via File > Open User Folder.

A patch is provided for use with Reggie! Next 4.9.0, extract the folder into the `/reggiedata/patches/` folder of Reggie! Next.

## The Codebase
As opposed to projects like NewerSMBW or NSMBWer+, PropelParts uses [Kamek 2](https://github.com/Treeki/Kamek) with the [Kamek Ninja Template](https://github.com/NSMBW-Community/Kamek-Ninja-Template),
along with headers from the [NSMBW Decomp](https://github.com/NSMBW-Community/NSMBW-Decomp). This is intended to provide a universal, modern basis for NSMBW custom code, as opposed to the base provided by NewerSMBW.

#### Setup
* Build [Kamek](https://github.com/Treeki/Kamek) and add the path to the folder containing the executable to the `KAMEK_PATH` environment variable. 
* Install a recent version of Python 3.
* Download [Ninja](https://ninja-build.org) and place it in the `/PropelParts/` directory
  * Alternatively, you may add Ninja to PATH
* Download the CodeWarrior for Embedded PowerPC binaries from [here](https://files.decomp.dev/compilers_20230715.zip) and extract it into the `/PropelParts/cw/` directory. The compiler should be located at `/compilers/Wii/1.3/mwcceppc.exe`.
  * The version of CodeWarrior used by Kamek's examples and NewerSMBW are not supported currently. This will change in the near future. 
* If you're on a non-Windows system, [Wine](https://www.winehq.org/) is needed to run CodeWarrior.
#### Building
Run `configure.bat` (or `configure.sh` if you're not on Windows) to configure ninja. Once you've ran the configuration script, run ninja. The project binaries should be located in the `/PropelParts/bin/` folder. 
For a more detailed explanation, check out the [Kamek Ninja Template](https://github.com/NSMBW-Community/Kamek-Ninja-Template) and it's documentation.
#### Contributing
Contributions are always welcome. Please see `CONTRIBUTING.md` for more details.
