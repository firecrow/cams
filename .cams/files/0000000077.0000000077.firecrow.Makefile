cams: cams.c cams.h utils.h utils.c 
	gcc -o cams cams.c utils.c

play: play.c utils.c utils.h
	gcc -o play play.c utils.c
