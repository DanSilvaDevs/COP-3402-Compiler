#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* errorCodes[] = {
  "", // This is just here so I don't have to index errors starting at 0
  "1. Use = instead of :=.",
  "2. = must be followed by a number.",
  "3. Identifier must be followed by =.",
  "4. const, var, procedure must be followed by identifier.",
  "5. Semicolon or comma missing.",
  "6. Incorrect symbol after procedure declaration.",
  "7. Statement expected.",
  "8. Incorrect symbol after statement part in block.",
  "9. Period expected.",
  "10. Semicolon between statements missing.",
  "11. Undeclared identifier.",
  "12. Assignment to constant or procedure is not allowed.",
  "13. Assignment operator expected.",
  "14. Call must be followed by an identifier.",
  "15. Call of a constant or variable is meaningless.",
  "16. then expected.",
  "17. Semicolon or } expected.",
  "18. do expected.",
  "19. Incorrect symbol following statement.",
  "20. Relational operator expected.",
  "21. Expression must not contain a procedure identifier.",
  "22. Right parenthesis missing.",
  "23. The preceding factor cannot begin with this symbol.",
  "24. An expression cannot begin with this symbol.",
  "25. This number is too large.",
  "26. Read and write must be followed by an identifier."
};

int main();
void readTokens();
void getToken();
void program();
void constant();
int variable();
void procedure();
void block();
void statement();
void expression();
void condition();
void term();
void factor();
int relation();

Symbol* findInTable(char *ident);
void insertSym(char* ident, int val, int kind);
void printSymbolsTable();
void error(int code);

#endif
