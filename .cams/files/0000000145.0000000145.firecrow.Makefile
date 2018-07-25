cams: tree cams.c cams.h
	cc -o cams cams.c -lcrowtils -L./crowtree -lcrowtree

install: cams
	cp ./cams /usr/bin/

clean:
	rm -f cams
	make clean -C crowtree

tree:
	make tree -C crowtree
