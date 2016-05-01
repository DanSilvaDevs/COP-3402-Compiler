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

// The index of instructions
int codeIndex = 0;

// The array of all instructions
Instruction* code[MAX_INSTRUCTIONS];

void parse() {
  // Read the tokens from lexemelist.txt into tokenList
  readTokens();

  // Parse the program!
  program();

  // Let the user know the program is grammatically correct.
  printf("No errors, program is syntactically correct.\n");

  // Print the symbol list to symlist.txt
  printSymbolsTable();

  // Print the instructions to mcode.txt
  printInstructions();
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

  // Add the halt instruction
  addInstruction(SIO3, 0, HALT);
}

void block() {
  // The space needed for the return value, SL, DL, and return address
  int space = 4;

  // Save the index of the jump instruction
  // so we can modify its `m` later.
  int jmpIndex = codeIndex;

  // Create the jump instruction.
  addInstruction(JMP, 0, 0);

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
    procedure(jmpIndex);
  }

  // Change the previously added JMP's `m`
  // to the current code index.
  code[jmpIndex]->m = codeIndex;

  // Allocate space for our variables
  addInstruction(INC, 0, space);

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
    insertSym(ident->val, atoi(token->val), constkind, 0);
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
    insertSym(token->val, -1, varkind, numVariables + 4);
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

void procedure(int jmpIndex) {
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
  insertSym(token->val, -1, prockind, jmpIndex + 1);

  // Modify the procedure's level and address.
  symbolTable[symbolIndex-1]->level = level;

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

  addInstruction(OPR, 0, RTN); 

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
    if (sym->kind != varkind) {
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

    // Store the result at the appropriate symbol's address
    addInstruction(STO, level - sym->level, sym->addr);
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
    if (sym->kind != prockind) {
      error(15);
    }

    // Add the instruction to jump to the procedure
    addInstruction(CAL, level, sym->addr);

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
    condition();

    // "if" must be followed by "then".
    if (token->type != thensym) {
      error(16);
    }

    // Save the index of our instructions to
    // modify the JPC later.
    int tempIfIndex = codeIndex;

    // Conditional jump. Decides if we do or don't skip the
    // `if` code.
    addInstruction(JPC, 0, 0);

    // Parse the statement following the "then"
    getToken();
    statement();

    // Modify the JPC's `m` to point to the new index
    code[tempIfIndex]->m = codeIndex;

    // Else group is optional.
    if (token->type == elsesym) {

      // Store the current code index again.
      // Used to modify the following JMP
      int tempElseIndex = codeIndex;

      // Add a JMP instruction
      addInstruction(JMP, 0, 0);

      getToken();
      statement();

      // Change the `m` of the JMP 
      // to be the current code index.
      code[tempElseIndex]->m = codeIndex;
    }
  }

  else if (token->type == whilesym) {
    // Store the current instruction index
    int tempIndex1 = codeIndex;

    // Parse the condition following the "while"
    condition();

    // Store the current instruction index
    int tempIndex2 = codeIndex;

    // Generate a JPC to potentially skip the contents of
    // the loop
    addInstruction(JPC, 0, 0);

    // If the condition isn't followed by "do",
    // throw an error.
    if (token->type != dosym) {
      error(18);
    }

    // Parse the statement after the "do"
    getToken();
    statement();

    // Add a JPC to potentially restart the loop
    addInstruction(JMP, 0, tempIndex1);

    // Modifies the JPC to skip to the current index if the condition isn't true
    code[tempIndex2]->m = codeIndex;
  }

  else if (token->type == readsym) {
    getToken();

    Symbol *sym = findInTable(token->val);
    if (!sym) {
      // Undeclared identifier found
      error(11);
    }

    // Read the input from the user
    addInstruction(SIO2, 0, READ);

    // If the identifier isn't a variable, throw an error
    if (sym->kind != varkind) {
      error(12);
    }

    // Store the data in the correct location
    addInstruction(STO, level - sym->level, sym->addr);

    getToken();
  }

  else if (token->type == writesym) {
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

    // If it's a variable, retrieve it from memory
    if (sym->kind == varkind) {
      addInstruction(LOD, level - sym->level, sym->addr);
    }

    // If it's a constant, push a literal onto the stack
    if (sym->kind == constkind) {
      addInstruction(LIT, 0, sym->val);
    }

    // Write the value to the screen
    addInstruction(SIO1, 0, WRITE);

    getToken();
  }
}

void condition() {
  // If the token is oddsym,
  // the condition is "oddsym expression"
  if (token->type == oddsym) {
    // Add the ODD instruction
    addInstruction(OPR, 0, ODD);

    getToken();
    expression();
  }

  // Otherwise, the condition is "expression rel-op expression"
  else {
    getToken();
    expression();

    int rel = relationType();

    // If we don't find a relation operator,
    // throw an error.
    if (!relation()) {
      error(20);
    }

    getToken();
    expression();

    // Add the appropriate OPR instruction
    addInstruction(OPR, 0, rel);
  }
}

void expression() {
  int op;
  // If we precede the term with a plus or minus, special handling is needed
  if (token->type == plussym || token->type == minussym) {
    op = token->type;

    getToken();
    term();

    // Negate the result if a minussym precedes it
    if (op == minussym) {
      addInstruction(OPR, 0, NEG);
    }
  }

  // Otherwise, just a parse a term
  else {
    term();
  }

  while (token->type == plussym || token->type == minussym) {
    op = token->type;

    getToken();
    term();

    if (op == plussym) {
      addInstruction(OPR, 0, ADD);
    }
    else {
      addInstruction(OPR, 0, SUB);
    }
  }
}

void term() {
  int op;

  // Parse the required factor.
  factor();

  // Continue to parse factors while the next symbol is a * or /
  while (token->type == multsym || token->type == slashsym) {
    op = token->type;

    getToken();
    factor();

    if (op == multsym) {
      addInstruction(OPR, 0, MUL);
    }
    else {
      addInstruction(OPR, 0, DIV);
    }
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

    // If it's a variable, load it from memory
    if (sym->kind == varkind) {
      addInstruction(LOD, level - sym->level, sym->addr);
    }

    // If it's a constant, use LIT
    else if (sym->kind == constkind) {
      addInstruction(LIT, 0, sym->val);
    }
  }

  // If we've got a number, we're good to go.
  // No further actions to take.
  else if (token->type == numbersym) {
    // Use LIT to push the number to the stack
    addInstruction(LIT, 0, atoi(token->val));

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

      // Check to see that the identifiers have the same name
      if (strcmp(ident, symbolTable[i]->name) == 0) {
        return symbolTable[i];
      }
    }
  }

  return NULL;
}

void insertSym(char *ident, int val, int kind, int addr) {
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
  sym->addr = addr;

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
      case constkind: fprintf(output, "%s\t", "const"); break;
      case varkind: fprintf(output, "%s\t", "var"); break;
      case prockind: fprintf(output, "%s\t", "proc"); break;
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

void addInstruction(int op, int l, int m) {
  Instruction *inst = (Instruction*)(malloc(sizeof(Instruction)));

  inst->op = op;
  inst->l = l;
  inst->m = m;

  code[codeIndex++] = inst;
}

void printInstructions() {
  FILE *out = fopen("mcode.txt", "w");
  int i;

  for (i = 0; i < codeIndex; i++) {
    Instruction *inst = code[i];
    fprintf(out, "%d %d %d\n", inst->op, inst->l, inst->m);
  }
  fclose(out);
}

int relationType() {
  return token->type - 1; /* 1 is the offset between the token's type and the OPR code */
}

void error(int code) {
  fprintf(stderr, "Error Code %s\n", errorCodes[code]);
  exit(EXIT_FAILURE);
}
