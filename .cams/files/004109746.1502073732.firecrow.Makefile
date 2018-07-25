MAIN_SRCS = cams.c commit.c diff.c ent.c list.c slist.c sync.c utils.c intls.c crowopt/opt.c 
LIBS = -lcrowtils -L./crowtree -lcrowtree 

cams: tree cams.c cams.h
	cc -o cams $(MAIN_SRCS) $(LIBS)

test: tree cams.c cams.h
	cc -o cams $(MAIN_SRCS) $(LIBS) -Ddebug
	./test.py
	make clean

unittest: tree cams.c cams.h
	cc -o cams $(MAIN_SRCS) $(LIBS) -DUNITTEST

install: cams
	cp ./cams /usr/bin/

clean:
	rm -f cams
	make clean -C crowtree

tree:
	make tree -C crowtree

tar:
	tar -czhf cams.`date +%F`.tar.tgz Makefile cams.c cams.h commit.c crowopt crowtree diff.c ent.c list.c sync.c test.py testing utils.c
