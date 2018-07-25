cams: tree cams.c cams.h
	cc -o cams cams.c -lcrowtils -L./crowtree -lcrowtree

test: tree cams.c cams.h
	cc -o cams cams.c -lcrowtils -L./crowtree -lcrowtree -Ddebug
	./test.py
	make clean

install: cams
	cp ./cams /usr/bin/

clean:
	rm -f cams
	make clean -C crowtree

tree:
	make tree -C crowtree
