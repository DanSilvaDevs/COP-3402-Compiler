#ifndef LEXER_H
#define LEXER_H

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "constants.h"
#include "structs.h"

int isletter(char c);
void cleanRawFile();
int parseWordType(char *val);
Token* parseWordToken(char cur, Token *token, FILE *input);
Token* parseNumericToken(char cur, Token *token, FILE *input);
Token* parseSpecialToken(char cur, Token *token, FILE *input);
void printLexemeList(int numTokens, Token *tokens[]);
void printLexemeTable(int numTokens, Token *tokens[]);

#endif
