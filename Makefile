cams: cams.c utils.c ent.c commit.c list.c slist.c
	cc -o ./bin/cams cams.c
clean:
	rm ./bin/cams
