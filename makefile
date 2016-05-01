default: compiler

compiler:
	gcc compiler.c -o compiler

parser:
	gcc parser.c -o parser

lexer:
	gcc lexer.c -o lexer

clean:
	rm -f compiler
	rm -f cleaninput.txt
	rm -f lexemelist.txt
	rm -f lexemetable.txt
	rm -f symlist.txt
	rm -f mcode.txt
	rm -f stacktrace.txt
