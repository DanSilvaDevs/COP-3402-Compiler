default: compiler

compiler:
	gcc compiler.c -o compiler

parser:
	gcc parser.c -o parser

lexer:
	gcc lexer.c -o lexer

clean:
	rm -f compiler
	rm -f parser
	rm -f lexer
	rm -f lexemelist.txt
	rm -f lexemetable.txt
	rm -f symlist.txt
	rm -f mcode.txt
	rm -f stacktrace.txt
