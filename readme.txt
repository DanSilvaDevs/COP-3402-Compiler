*** Compilation Instructions ***
1. Unzip `compiler.zip` by typing `unzip compiler.zip`.
2. To compile the compiler, simply type `make`.
3. To run the compiler, type `./compiler`.
4. To remove all output files and start again, type `make clean ; make`,
   which will remove `cleaninput.txt`, `lexemelist.txt`, `lexemetable.txt`,
   `mcode.txt`, `stacktrace.txt`, and `compiler`, and then recompile `compiler`.

*** Compiler Run Details ***
1. The compiler reads the input file named `input.txt`.
2. It will generate a file named `cleaninput.txt`, which will be the same as
   `intput.txt`, but without comments.
3. It will lexically analyze `cleaninput.txt` and generate `lexemelist.txt`
   and `lexemetable.txt`.
4. It will parse the program using `lexemelist.txt` and generate code for the
   program, which will be stored in `mcode.txt`.
5. Lastly, it will run the PM/0 code and store the stack trace in `stacktrace.txt`.
