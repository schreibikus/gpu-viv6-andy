#!/bin/bash

if [ -z "$AQROOT" ]; then
   AQROOT=$PWD/../../..
fi
GC_GLSL_PATH=$AQROOT/compiler/libGLSLC/compiler

#
# generate gc_glsl_scanner.c, gc_glsl_parser.c, and gc_glsl_token_def.h
#
#cd $GC_GLSL_PATH; flex -L gc_glsl.l
cd $GC_GLSL_PATH; flex  gc_glsl.l

GC_GLSL_SCANNER_C=$GC_GLSL_PATH/gc_glsl_scanner.c
GC_GLSL_PARSER_C=$GC_GLSL_PATH/gc_glsl_parser.c

#
# clean up the stuff about libc in gc_glsl_scanner.c
#
if [ -e "$GC_GLSL_SCANNER_C" ]; then
	sed -i "s/#include.*stdio.h.*/#include \"gc_glsl_scanner.h\"/" $GC_GLSL_SCANNER_C
	sed -i "s/.*string.h.*//" $GC_GLSL_SCANNER_C
	sed -i "s/.*errno.h.*//" $GC_GLSL_SCANNER_C
	sed -i "s/.*unistd.h.*//" $GC_GLSL_SCANNER_C
	sed -i "s/.*stdlib.h.*//" $GC_GLSL_SCANNER_C
	sed -i "s/.*<id.h>.*//" $GC_GLSL_SCANNER_C
	sed -i "s/.*fprint.*stderr.*msg.*//" $GC_GLSL_SCANNER_C
	sed -i "s/exit.*YY_EXIT_FAILURE.*/slReport(0, 0, slvREPORT_FATAL_ERROR, (char \*)msg);/" $GC_GLSL_SCANNER_C
	sed -i "s/\([ \t\r]\)malloc\([ \t]*(\)/\1slMalloc\2/" $GC_GLSL_SCANNER_C
	sed -i "s/\([ \t\r]\)free\([ \t]*(\)/\1slFree\2/" $GC_GLSL_SCANNER_C
	sed -i "s/\([ \t\r]\)realloc\([ \t]*(\)/\1slRealloc\2/" $GC_GLSL_SCANNER_C
	#sed -i "s/return.*[\r\n]*[ \t]*YY_BREAK/return&/" $GC_GLSL_SCANNER_C
	sed -i "s/return\(.*[\r\n]*[ \t]*\)YY_BREAK/return\1/" $GC_GLSL_SCANNER_C
else
	echo
	echo ERROR: not found $GC_GLSL_SCANNER_C
	echo
fi

# Use -l to disable generating #line directive
cd $GC_GLSL_PATH; bison -t -v -d gc_glsl.y
cd $GC_GLSL_PATH; echo "#ifndef __gc_glsl_token_def_h_" > gc_glsl_token_def.h
cd $GC_GLSL_PATH; echo "#define __gc_glsl_token_def_h_" >> gc_glsl_token_def.h
cd $GC_GLSL_PATH; cat gc_glsl.tab.h >> gc_glsl_token_def.h; rm -f gc_glsl.tab.h
cd $GC_GLSL_PATH; echo "#endif /* __gc_glsl_token_def_h_ */" >> gc_glsl_token_def.h
cd $GC_GLSL_PATH; sed -i "s/gc_glsl.tab.h/gc_glsl_token_def\.h/" gc_glsl_token_def.h
cd $GC_GLSL_PATH; mv gc_glsl.tab.c gc_glsl_parser.c
cd $GC_GLSL_PATH; sed -i "s/gc_glsl.tab.c/gc_glsl_parser\.c/" gc_glsl_parser.c

#
# clean up the stuff about libc in gc_glsl_parser.c
#
if [ -e "$GC_GLSL_PARSER_C" ]; then
    cp $GC_GLSL_PARSER_C ${GC_GLSL_PARSER_C}.orig
	sed -i "s/#include.*gc_glsl_parser.h.*/#include \"gc_glsl_parser.h\"/" $GC_GLSL_PARSER_C
	sed -i "s/.*stdlib.h.*//" $GC_GLSL_PARSER_C
	sed -i "s/.*stdio.h.*//" $GC_GLSL_PARSER_C
	sed -i "s/\([ \t\r]\)malloc$/\1slMalloc/" $GC_GLSL_PARSER_C
	sed -i "s/\([ \t\r]\)malloc\([ \t\r]*(\)/\1slMalloc\2/" $GC_GLSL_PARSER_C
	sed -i "s/\([ \t\r]\)free$/\1slFree/" $GC_GLSL_PARSER_C
	sed -i "s/\([ \t\r]\)\(free(\[ \t\r]*(\)/\1slFree\2/" $GC_GLSL_PARSER_C
	sed -i "s/\([ \t\r]\)realloc$/\1slRealloc/" $GC_GLSL_PARSER_C
	sed -i "s/\([ \t\r]\)\(realloc[ \t\r]*(\)/\1slRealloc\2/" $GC_GLSL_PARSER_C
	sed -i "s/\([ \t\r]\)alloca$/\1slMalloc/" $GC_GLSL_PARSER_C
	sed -i "s/\([ \t\r]\)\(alloca[ \t\r]*(\)/\1slMalloc\2/" $GC_GLSL_PARSER_C
else
	echo
	echo ERROR: not found $GC_GLSL_PARSER_C
	echo
fi

