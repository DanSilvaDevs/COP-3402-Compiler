HEADERS = parser.h

default: program

program:
	gcc parser.c -o parser

clean:
	rm -f parser
	rm -f symlist.txt
