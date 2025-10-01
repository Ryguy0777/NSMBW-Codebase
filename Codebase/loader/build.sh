if [ "$1" = "--wine" ]; then
    CC='wine ../cw/mwcceppc.exe'
else
    CC='../cw/mwcceppc.exe'
fi

$CC -i . -I- -i ../k_stdlib -Cpp_exceptions off -enum int -Os -use_lmw_stmw on -fp hard -rostr -sdata 0 -sdata2 0 -c -o kamekLoader.o kamekLoader.cpp
$CC -i . -I- -i ../k_stdlib -Cpp_exceptions off -enum int -Os -use_lmw_stmw on -fp hard -rostr -sdata 0 -sdata2 0 -c -o nsmbw.o nsmbw.cpp

$KAMEK_PATH kamekLoader.o nsmbw.o -static=0x800046E4 -output-code=loader.bin -output-riiv=loader.xml -valuefile=Code/loader.bin

# Or to inject directly into a DOL:
# $KAMEK_PATH kamekLoader.o nsmbw.o -static=0x800046E4 -input-dol=main.dol -output-dol=main_kamek.dol
