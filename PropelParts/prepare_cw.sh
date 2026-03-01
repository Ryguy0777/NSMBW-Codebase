#!/bin/bash

set -e
dir=$(mktemp -d)

echo "Downloading NXP CodeWarrior Special Edition for MPC55xx/MPC56xx v2.10..."
curl -L -o $dir/CW55xx_v2_10_SE.exe https://www.nxp.com/lgfiles/devsuites/PowerPC/CW55xx_v2_10_SE.exe

# Extract the subfiles from the installer using 7-Zip
7z x -o$dir $dir/CW55xx_v2_10_SE.exe
7z x -o$dir $dir/Data1.cab _44CD8EA541F44E56801091477F3DC9AA
7z x -o$dir $dir/Data1.cab _274B47372A7D4391B92F01E51B171A58
7z x -o$dir $dir/Data1.cab _4C8ADA37887647F5955B4FB0F716277F
7z x -o$dir $dir/Data1.cab _C63F22BC0503480DAFDCAFC394185CBA
7z x -o$dir $dir/Data1.cab _4901FAD667D24E13BCF700F695CDD6D7
mkdir -p cw
mv $dir/_44CD8EA541F44E56801091477F3DC9AA cw/license.dat
mv $dir/_274B47372A7D4391B92F01E51B171A58 cw/lmgr11.dll
mv $dir/_4C8ADA37887647F5955B4FB0F716277F cw/mwcceppc.exe
mv $dir/_C63F22BC0503480DAFDCAFC394185CBA cw/mwasmeppc.exe
mv $dir/_4901FAD667D24E13BCF700F695CDD6D7 cw/mwldeppc.exe
rm -r $dir

# Append extra license increments to the license file
cat <<EOL >> cw/license.dat
INCREMENT Win32_Plugins_Link_PPC metrowks 8.0 permanent uncounted \\
        385AF433B467 VENDOR_STRING="PPC linker" HOSTID=ANY TS_OK
INCREMENT Win32_Plugins_PPC_Nintendo metrowks 8.0 permanent uncounted \\
        E6C34EF875C0 VENDOR_STRING="PPC Gekko Plugins" HOSTID=ANY \\
        TS_OK
EOL
