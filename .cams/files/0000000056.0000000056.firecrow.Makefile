cams: cams.c
	gcc -o cams cams.c
play: play.c utils.c utils.h
	gcc -o play play.c utils.c
