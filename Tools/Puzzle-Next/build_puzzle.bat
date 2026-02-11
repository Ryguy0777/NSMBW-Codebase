@echo off
echo NOTE: Having upx downloaded and added to PATH can help reduce the size.
echo HUGE credit to RoadrunnerWMC, we used his building script as a base.
echo
echo Older PyInstaller command line:
echo pyinstaller --upx-dir=/path/to/upx --windowed -y --onefile reggie.py
echo
py -3 -OO build_puzzle.py
pause