D:/Github/Kamek/cw/mwcceppc.exe -i . -I- -i ../k_stdlib -Cpp_exceptions off -enum int -Os -use_lmw_stmw on -fp hard -rostr -sdata 0 -sdata2 0 -c -o kamekLoader.o kamekLoader.cpp
D:/Github/Kamek/cw/mwcceppc.exe -i . -I- -i ../k_stdlib -Cpp_exceptions off -enum int -Os -use_lmw_stmw on -fp hard -rostr -sdata 0 -sdata2 0 -c -o nsmbw.o nsmbw.cpp

D:/Github/Kamek/Kamek/bin/Debug/net6.0/Kamek kamekLoader.o nsmbw.o -static=0x800046E4 -output-code=loader.bin -output-riiv=loader.xml -valuefile=Code/loader.bin

