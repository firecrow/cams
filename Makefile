SHELL = /usr/local/bin/bash
cams: 
	cc -o ./bin/cams cams.c
clean:
	rm ./bin/cams
