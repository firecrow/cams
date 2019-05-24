SHELL = /usr/local/bin/bash
cams: 
	cc -o ./bin/cams cams.c

unit:
	cc -o ./bin/test.utils ./test/utils.test.c

clean:
	rm ./bin/cams
