cams: cams.c obj/utils.c obj/ent.c obj/commit.c obj/list.c obj/slist.c
	cc -o ./bin/cams cams.c
clean:
	rm ./bin/cams
