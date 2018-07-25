cams: cams.c cams.h utils.h utils.c 
	gcc -o cams cams.c utils.c

play: play.c utils.c utils.h tree.c tree.h
	gcc -o play play.c utils.c tree.c

treeprint: treeprint.c tree.c tree.h
	gcc -o treeprint treeprint.c tree.c utils.c

install: cams
	cp ./cams /usr/bin/
