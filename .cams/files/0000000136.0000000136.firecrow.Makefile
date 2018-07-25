cams: cams.c cams.h 
	cc -o cams cams.c -lcrowtils

install: cams
	cp ./cams /usr/bin/
