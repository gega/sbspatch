#
all:	sbsp sbsdiff

sbsp:	sbsp.h sbsp.c
	gcc -Wall -o sbsp sbsp.c

sbsdiff:	sbsdiff.c
		gcc -Wall -DBSDIFF_EXECUTABLE -o sbsdiff sbsdiff.c

test:	sbsp sbsdiff
	./test.sh 100
