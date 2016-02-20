#@A (C) 1992 Allen I. Holub 
# Make both models of compiler with borland.mak or nmake. This is a On-Command!
# shell script for those of you who are using that shell. You can translate
# to COMMAND.COM by extracting the two lines that invoke make and changing
# the slashes to backslashes.
echo -e making SMALL-model libraries

set BOR_S = make -fborland.mak -DMODEL=s '-DCTARG=\lib\comp' '-DLTARG=\lib\l'
set MICRO_S = nmake MODEL=-AS 'CTARG=/lib/comp.lib' 'LTARG=/lib/l.lib'
set domake_s = $MICRO_S

set BOR_C = make -fborland.mak -DMODEL=c '-DCTARG=\lib\compc' '-DLTARG=\lib\lc'
set MICRO_C = nmake MODEL=-AC 'CTARG=/lib/compc.lib' 'LTARG=/lib/lc.lib'
set domake_c = $MICRO_C

mkdir small
mkdir compact
/bin/new/mv.exe -e small/*.* .
$domake_s
if( $status == 0 ) then
    /bin/new/mv.exe -e *.obj small
    echo -e making COMPACT-model libraries
    /bin/new/mv.exe -e compact/*.* .
    if( $status == 0 ) then
        $domake_c
	/bin/new/mv.exe -e *.obj compact
    endif
endif
