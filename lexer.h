#ifndef LEXER_H
#define LEXER_H

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define MAX_IDENT_LEN 11
#define MAX_NUM_LEN 5
#define MAX_TOKENS 2000

#define RAW_INPUT_PATH "input.txt"
#define CLEAN_INPUT_PATH "cleaninput.txt"
#define LEXEME_TABLE_PATH "lexemetable.txt"
#define LEXEME_LIST_PATH "lexemelist.txt"

typedef struct {
  int type;
  char val[MAX_IDENT_LEN + 1];
} Token;

typedef enum {
  nulsym = 1, identsym, numbersym, plussym, minussym,
  multsym,  slashsym, oddsym, eqsym, neqsym, lessym, leqsym,
  gtrsym, geqsym, lparentsym, rparentsym, commasym, semicolonsym,
  periodsym, becomessym, beginsym, endsym, ifsym, thensym,
  whilesym, dosym, callsym, constsym, varsym, procsym, writesym,
  readsym , elsesym, unknownsym } token_type;

int isletter(char c);
void cleanRawFile();
int parseWordType(char *val);
Token* parseWordToken(char cur, Token *token, FILE *input);
Token* parseNumericToken(char cur, Token *token, FILE *input);
Token* parseSpecialToken(char cur, Token *token, FILE *input);
void printLexemeList(int numTokens, Token *tokens[]);
void printLexemeTable(int numTokens, Token *tokens[]);

#endif
