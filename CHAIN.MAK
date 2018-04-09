
chain.obj: chain.c chain.h
    cl -c -Alfu -G2s -Zi chain.c

minsubs.obj: minsubs.c chain.h
    cl -c -Alfu -G2s -Zi minsubs.c

chain.res: chain.rc chain.h
    rc -r chain

chain.exe:  chain.obj minsubs.obj chain.def
    link /CO chain minsubs, /align:16, nul, /nod llibcmt os2, chain
    rc chain.res chain.exe

chain.exe : chain.res
    rc chain.res chain.exe

            


















