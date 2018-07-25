cams: tree cams.c cams.h
	cc -o cams cams.c commit.c diff.c ent.c list.c slist.c sync.c utils.c crowopt/opt.c -lcrowtils -L./crowtree -lcrowtree

test: tree cams.c cams.h
	cc -o cams cams.c commit.c diff.c ent.c list.c slist.c sync.c utils.c crowopt/opt.c -lcrowtils -L./crowtree -lcrowtree -Ddebug
	./test.py
	make clean

install: cams
	cp ./cams /usr/bin/

clean:
	rm -f cams
	make clean -C crowtree

tree:
	make tree -C crowtree

tar:
	tar -czhf cams.`date +%F`.tar.tgz Makefile cams.c cams.h commit.c crowopt crowtree diff.c ent.c list.c sync.c test.py testing utils.c

