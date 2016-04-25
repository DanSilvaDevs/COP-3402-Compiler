#include "constants.h"
#include "structs.h"
#include "parser.h"

// The array containing tokens from lexemelist.txt
Token* tokenList[MAX_INSTRUCTIONS];
int tokenCount = 0;
int tokenIndex = 0;

// The current token being examined
Token* token;

// An array containing the name of the procedure
// at each lexical level
char scopes[MAX_LEXI_LEVEL][MAX_IDENT_LEN+1];

// Contains all symbols in the symbol table.
Symbol* symbolTable[MAX_SYMBOL_TABLE_SIZE];
int symbolIndex = 0;

// The current block level
int level = 0;

int main() {
  // Read the tokens from lexemelist.txt into tokenList
  readTokens();

  // Parse the program!
  program();

  // Let the user know the program is grammatically correct.
  printf("No errors, program is syntactically correct.");

  // Print the symbol list to symlist.txt
  printSymbolsTable();
}

void program() {
  // Parse the required block.
  getToken();
  block();

  // If our last token isn't a period,
  // throw an error
  if (token->type != periodsym) {
    error(9);
  }
}

void block() {
  // The space needed for the return value, SL, DL, and return address
  int space = 4;

  // Parse constants first.
  if (token->type == constsym) {
    constant();
  }

  // Parse variables second.
  if (token->type == varsym) {

    // Make sure to allocate space for each of the variable declarations
    space += variable();
  }

  // Parse procedures last
  while (token->type == procsym) {
    procedure();
  }

  // Parse a statement next, after we're done defining
  // constants, vars, and procedures
  statement();
}

void constant() {

  // Constants are of the form
  // ident = number(, ident = number)*;
  do {
    getToken();
    // Store the identifier so we can insert the constant into the
    // symbol table.
    Token* ident = token;
    if (token->type != identsym) {
      error(4);
    }

    // Identifiers must be followed by '='
    getToken();
    if (token->type != eqsym) {
      error(3);
    }

    // '=' must be followed by a number.
    getToken();
    if (token->type != numbersym) {
      error(2);
    }

    // Insert the constant into our symbol table
    insertSym(ident->val, atoi(token->val), consttype);
    getToken();
  } while (token->type == commasym);

  // Constant declarations must end with a semicolon.
  // If one is not found, throw an error.
  if (token->type != semicolonsym) {
    error(10);
  }

  getToken();
}

int variable() {
  // The number of variables declared.
  // We need this to know how much space to allocate
  // for the variables we're storing.
  int numVariables = 0;

  // Variables are of the form ident(, ident)*;
  do {
    // We're expecting an identifier; if we don't find one
    // we need to throw an error
    getToken();
    if (token->type != identsym) {
      error(4);
    }

    // Insert our variable into the symbol table
    // We use numVariables + 4 to indicate its position
    // in the current AR; starting at 4 since its offset is
    // 4 by default.
    insertSym(token->val, numVariables + 4, vartype);
    getToken();

    // Create space for each of the variables
    // that need to be stored
    numVariables += 1;
  } while (token->type == commasym);

  // If our variable declarations don't
  // end with a semicolon, throw an error
  if (token->type != semicolonsym) {
    error(10);
  }
  getToken();

  // Return the number of declarations
  // so we can allocate space for them in
  // the block parsing
  return numVariables;
}

void procedure() {
  // If 'procedure' isn't followed by an identifier, throw an error.
  getToken();
  if (token->type != identsym) {
    error(4);
  }

  // Store the identifier of the current procedure.
  // This is used to implement namespacing so that
  // variables cannot be used outside of their scope.
  strcpy(scopes[level], token->val);

  // Insert our procdure's identifier into the symbol table.
  // Because we aren't generating code, I'm settings its val
  // to -1.
  insertSym(token->val, -1, proctype);

  // Increase the level by one
  ++level;

  // If our procedure declaration isn't followed by a semicolon,
  // throw an error.
  getToken();
  if (token->type != semicolonsym) {
    error(6);
  }

  // After a procedure declaration, a block follows.
  getToken();
  block();

  // If the block is not followed by a semicolon, throw an error
  if (token->type != semicolonsym) {
    error(10);
  }

  getToken();

  // We're done with this procedure so we're going up a level
  --level;
}

void statement() {
  // If an identifier is found,
  // we're looking for an assignment.
  if (token->type == identsym) {
    // If the identifier is not in our table, throw an error
    Symbol* sym = findInTable(token->val);
    if (!sym) {
      error(11);
    }

    // If the symbol is not a variable
    // (it is a constant or procedure) throw an error
    if (sym->kind != vartype) {
      error(12);
    }

    // If the next token is not :=
    // throw an error.
    getToken();
    if (token->type != becomessym) {

      // If the token is =, alert the user that the wrong
      // symbol was used.
      if (token->type == eqsym) {
        error(1);
      }

      error(19);
    }

    // Parse the expression that follows :=
    getToken();
    expression();
  }

  // If 'call' is found, we're looking for
  // a procedure identifier
  else if (token->type == callsym) {

    // If the identifier is not in our table, throw an error
    getToken();
    Symbol* sym = findInTable(token->val);
    if (!sym) {
      error(11);
    }

    // If "call" is not followed by an identifier,
    // throw an error.
    if (token->type != identsym) {
      error(14);
    }

    // If the symbol is not a procedure
    // (it is a constant or variable) throw an error
    if (sym->kind != proctype) {
      error(15);
    }

    getToken();
  }
  else if (token->type == beginsym) {
    getToken();
    statement();

    // If the next sym isn't a semicolon and isn't an end,
    // throw an error
    if (token->type != semicolonsym && token->type != endsym) {
      error(10);
    }

    // Statements are separated by semicolons.
    // Continue while the current statement ends with a semicolon.
    while (token->type == semicolonsym) {
      getToken();
      statement();

      // If the next sym isn't a semicolon and isn't an end,
      // throw an error
      if (token->type != semicolonsym && token->type != endsym) {
        error(10);
      }
    }

    // If the statement doesn't end with "end",
    // throw an error.
    if (token->type != endsym) {
      error(8);
    }

    getToken();
  }
  else if (token->type == ifsym) {

    // Parse the conditional for the "if" statement.
    getToken();
    condition();

    // "if" must be followed by "then".
    if (token->type != thensym) {
      error(16);
    }

    // Parse the statement following the "then"
    getToken();
    statement();

    // Else group is optional.
    if (token->type == elsesym) {
      getToken();
      statement();
    }
  }
  else if (token->type == whilesym) {
    // Parse the condition following the "while"
    condition();

    // If the condition isn't followed by "do",
    // throw an error.
    if (token->type != dosym) {
      error(18);
    }

    // Parse the statement after the "do"
    getToken();
    statement();
  }
  else if (token->type == writesym || token->type == readsym) {
    getToken();

    // Read/write must be followed by an identifier.
    if (token->type != identsym) {
      error(26);
    }

    // If the identifier doesn't exist,
    // throw an error
    Symbol *sym = findInTable(token->val);
    if (!sym) {
      // Undeclared identifier found
      error(11);
    }

    getToken();
  }
}

void condition() {
  // If the token is oddsym,
  // the condition is "oddsym expression"
  if (token->type == oddsym) {
    getToken();
    expression();
  }

  // Otherwise, the condition is "expression rel-op expression"
  else {
    expression();

    // If we don't find a relation operator,
    // throw an error.
    if (!relation()) {
      error(20);
    }

    getToken();
    expression();
  }
}

void expression() {
  // Parse the required term.
  term();

  // Continue to parse terms while the next symbol is a + or -
  while (token->type == plussym || token->type == minussym) {
    getToken();
    term();
  }
}

void term() {
  // Parse the required factor.
  factor();

  // Continue to parse factors while the next symbol is a * or /
  while (token->type == multsym || token->type == slashsym) {
    getToken();
    factor();
  }
}

void factor() {
  // If we have an identifier, make sure it's declared
  // in an appropriate scope.
  if (token->type == identsym) {
    Symbol *sym = findInTable(token->val);
    if (!sym) {
      error(11);
    }
    getToken();
  }

  // If we've got a number, we're good to go.
  // No further actions to take.
  else if (token->type == numbersym) {
    getToken();
  }

  // If we have a left parenthesis,
  // parse the expression inside and
  // look for its matching right parenthesis.
  else if (token->type == lparentsym) {
    getToken();
    expression();

    // If there is no matching right parenthesis,
    // it is a malformed expression so we throw
    // an error.
    if (token->type != rparentsym) {
      error(22);
    }

    getToken();
  }

  // Otherwise, we know the parent expression cannot begin
  // with the current token.
  else {
    error(24);
  }
}

// Returns 1 if the token's type
// is a relation symbol (> >= < <= = !=).
// 0 otherwise.
int relation() {
  return (token->type >= eqsym && token->type <= geqsym);
}

Symbol* findInTable(char *ident) {
  int i;

  // Traverse the symbols in reverse order
  // to find the symbol in the closest scope.
  for (i = symbolIndex - 1; i >= 0; i--) {

    // Check to see that the identifiers are in the same
    // or higher schope.
    if (symbolTable[i]->level <= level) {
      // If the level is the same, make sure that the current procedure
      // is the same as the procedure this is defined in.
      if (strcmp(symbolTable[i]->procIdent, scopes[level]) != 0) {
        continue;
      }

      // Check to see that the identifiers have the same name
      if (strcmp(ident, symbolTable[i]->name) == 0) {
        return symbolTable[i];
      }
    }
  }

  return NULL;
}

void insertSym(char *ident, int val, int kind) {
  // Return if the symbol already exists in the table
  if (findInTable(ident)) {
    return;
  }

  // Create the new symbol to store in our symbol table.
  // We default the procIdent to "" so we don't get a segfault
  // later when we read its value.
  Symbol* sym = (Symbol*)(malloc(sizeof(Symbol)));
  strcpy(sym->name, ident);
  strcpy(sym->procIdent, "");
  sym->val = val;
  sym->kind = kind;
  sym->level = level;

  // If the symbol is a variable/constant,
  // store the name of the procedure it is defined in.
  if (kind != procsym) {
    strcpy(sym->procIdent, scopes[level]);
  }

  symbolTable[symbolIndex++] = sym;
}

void readTokens() {
  FILE *input = fopen("lexemelist.txt", "r");
  while (!feof(input)) {
    Token *curToken = (Token*)malloc(sizeof(Token));

    // Copy the default value of "" to the token value.
    // This ensures that we won't segfault by trying to read
    // an uninitialized value later on.
    strcpy(curToken->val, "");

    // Read the next token type from the lexeme list.
    fscanf(input, "%d ", &curToken->type);

    // Print the current token to the user.
    printf("%d ", curToken->type);

    // If the token type is identsym or numbersym,
    // read the next token as well, as we need the
    // value associated with the symbol type
    if (curToken->type == identsym || curToken->type == numbersym) {
      fscanf(input, "%[^ \n]s", curToken->val);

      // Print the current token to the user.
      printf("%s ", curToken->val);
    }

    tokenList[tokenCount++] = curToken;
  }

  // Print a newline for formatting sake
  printf("\n\n");
}

void getToken() {
  // Set the global token pointer to the next token in the array.
  token = tokenList[tokenIndex++];

  // If the token is NULL, we just initialize an empty token.
  // This is so we don't get a segfault later on if we read NULL token.
  if (!token) {
    token = (Token*)(malloc(sizeof(Token)));
    strcpy(token->val, "");
    token->type = unknownsym;
  }
}

void printSymbolsTable() {
  FILE *output = fopen("symlist.txt" ,"w");
  Symbol* currentSym;
  int i;

  fprintf(output, "Name\tType\tLevel\tValue\n");
  for (i = 0; i < symbolIndex; i++) {
    currentSym = symbolTable[i];

    // Print the symbol's name
    fprintf(output, "%s\t", currentSym->name);

    // Print the symbol's type
    switch (currentSym->kind) {
      case consttype: fprintf(output, "%s\t", "const"); break;
      case vartype: fprintf(output, "%s\t", "var"); break;
      case proctype: fprintf(output, "%s\t", "proc"); break;
      default: fprintf(output, "type is %d\t", currentSym->kind); exit(0);
    }

    // Print the level
    fprintf(output, "%d\t", currentSym->level);

    // Print the val unless it's a procedure,
    // meaning val == -1
    if (currentSym->val == -1){
      fprintf(output, "\n");
    } else {
      fprintf(output, "%d\n", currentSym->val);
    }
  }
}

void error(int code) {
  fprintf(stderr, "Error Code %s\n", errorCodes[code]);
  exit(EXIT_FAILURE);
}
