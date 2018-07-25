cams: tree cams.c cams.h
	cc -o cams cams.c commit.c utils.c ent.c sync.c list.c diff.c crowopt/opt.c -lcrowtils -L./crowtree -lcrowtree

test: tree cams.c cams.h
	cc -o cams cams.c commit.c utils.c ent.c sync.c list.c diff.c crowopt/opt.c -lcrowtils -L./crowtree -lcrowtree -Ddebug
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
