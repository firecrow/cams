MAIN_SRCS = cams.c commit.c diff.c ent.c list.c slist.c sync.c utils.c intls.c ../crowopt/opt.c 
LIBS = -lcrowtils -L../crowtree -lcrowtree  -I../ -I../crowtils -L../crowtils
UNITLIBS = -I../crowsuite -L../crowsuite -lcrowsuite 

cams: cams.c cams.h list.c commit.c sync.c slist.c
	cc -o cams $(MAIN_SRCS) $(LIBS)

test: cams.c cams.h
	cc -o cams $(MAIN_SRCS) $(LIBS) -Ddebug
	./test.py
	make clean

unittest: cams.c cams.h
	cc -o camsunit unittest.c $(MAIN_SRCS) $(LIBS) $(UNITLIBS) -DUNITTEST


install: cams
	cp ./cams /usr/bin/

clean:
	rm -f cams

tar:
	tar -czhf cams.`date +%F`.tar.tgz Makefile cams.c cams.h commit.c crowopt crowtree diff.c ent.c list.c sync.c test.py testing utils.c
