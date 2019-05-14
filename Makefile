cams: cams.c utils.c ent.c commit.c list.c slist.c
	cc -o cams cams.c
clean:
	rm ./cams
