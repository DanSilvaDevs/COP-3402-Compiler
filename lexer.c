#include "lexer.h"

int main() {
  Token *token;
  Token *tokens[MAX_TOKENS];
  int tokenCount = 0;
  char cur;

  // First, we clean our raw input file.
  // This removes all comments and places
  // the cleaned file in cleaninput.txt
  cleanRawFile();

  // Open the cleaned input file.
  // This is the file we're going to parse
  FILE *input = fopen(CLEAN_INPUT_PATH, "r");

  // Loop through all characters in the file
  // until we run out.
  while (!feof(input)) {

    // Reset our current token for the current pass
    token = malloc(sizeof(Token));
    token->type = unknownsym;
    memset(token->val, 0, sizeof(token->val));

    // Fetch the next char from the file
    cur = getc(input);

    // If that's our last character, we should exit the loop
    if (feof(input)) break;

    // We don't care about whitespace when tokenizing
    // so we skip them.
    if (isspace(cur)) continue;

    // We're parsing a token that begins with a letter.
    // This is either a reserved word or an identifer.
    if (isletter(cur)) {
      token = parseWordToken(cur, token, input);
    }

    // We're parsing a number token.
    else if (isdigit(cur)) {
      token = parseNumericToken(cur, token, input);
    }

    // Otherwise, we're handling a special character.
    else {
      token = parseSpecialToken(cur, token, input);
    }

    // Add our symbol to our symbol table
    tokens[tokenCount++] = token;
  }
  fclose(input);

  // Print our lexeme table to lexemetable.txt
  printLexemeTable(tokenCount, tokens);

  // Print our lexeme list to lexemelist.txt
  printLexemeList(tokenCount, tokens);
}

// Returns 1 if `c` is in { a...Z } or is an underscore.
// Returns 0 otherwise.
int isletter(char c) {
  return isalpha(c) || c == '_';
}

// Returns a token resulting from the parsing of a
// reserved word or an identifier.
// Raises an exception if the identifier is longer than 11 chars.
Token* parseWordToken(char cur, Token *token, FILE *input) {
  int tokenLength = 0;
  int tokenTooLong = 0;

  while (!feof(input) && (isletter(cur) || isdigit(cur))) {
    token->val[tokenLength++] = cur;

    // If our token is too long, set the flag so we throw an
    // error. We don't throw it now so we can show them the full token
    if (tokenLength > MAX_IDENT_LEN) {
      tokenTooLong = 1;
    }

    // If we find a digit, we know that the token is
    // an identifier, as reserved words don't have numbers.
    if (isdigit(cur)) {
      token->type = identsym;
    }

    // Fetch our next token and restart the loop
    cur = getc(input);
  }

  // If our token was too long, let the user know and exit.
  if (tokenTooLong) {
    fprintf(stderr, "ERROR: Identifier \"%s\" too long.\n", token->val);
    exit(EXIT_FAILURE);
  }

  // If we haven't determined that it is an identifier
  // by encountering a digit, we must determine
  // the token's actual type.
  if (token->type == unknownsym) {
    token->type = parseWordType(token->val);
  }

  // Since our last char wasn't part of
  // this token, we put it back on
  // the buffer.
  ungetc(cur, input);

  return token;
}

// Returns a token resulting from the parsing of a
// number. Its type will be `numbersym`.
// Raises an exception if the number is longer than 5 digits,
// or if there are any letters detected.
Token* parseNumericToken(char cur, Token *token, FILE *input) {
  int tokenTooLong = 0;
  int invalidIdent = 0;
  int tokenLength = 0;
  token->type = numbersym;

  while (!feof(input) && isdigit(cur)) {
    // Append the current letter to our current token
    // and fetch the next character from the file
    token->val[tokenLength++] = cur;
    cur = getc(input);

    // If we find a letter, we know it is an invalid
    // identifier, as identifiers cannot start with numbers
    if (isletter(cur)) {
      invalidIdent = 1;
    }

    // If our token is too long, set the flag so we throw an
    // error. We don't throw it now so we can show them the full token
    if (tokenLength > MAX_NUM_LEN) {
      tokenTooLong = 1;
    }
  }

  // If our token was too long, let the user know and exit.
  if (tokenTooLong) {
    fprintf(stderr, "ERROR: Number \"%s\" too long.\n", token->val);
    exit(EXIT_FAILURE);
  }

  // If we found a letter, we found an identifier, and
  // we should exit and alert the user.
  if (invalidIdent) {
    fprintf(stderr, "ERROR: Identifier \"%s\" cannot start with numbers.\n", token->val);
    exit(EXIT_FAILURE);
  }

  // Since our last char wasn't part of
  // this token, we put it back on
  // the buffer.
  ungetc(cur, input);

  return token;
}

// Returns a token resulting from the parsing of a
// special character.
// Raises an exception if an invalid symbol is encountered.
Token* parseSpecialToken(char cur, Token *token, FILE *input) {
  int tokenLength = 0;
  token->val[tokenLength++] = cur;

  switch (cur) {
    case '+':
      token->type = plussym;
      break;
    case '-':
      token->type = minussym;
      break;
    case '/':
      token->type = slashsym;
      break;
    case '*':
      token->type = multsym;
      break;
    case '(':
      token->type = lparentsym;
      break;
    case ')':
      token->type = rparentsym;
      break;
    // We can either have just '>'
    // or we can have '>='
    case '>':
      token->type = gtrsym;
      cur = getc(input);
      if (cur == '='){
        token->type = geqsym;
        token->val[tokenLength++] = cur;
      }
      else {
        ungetc(cur, input);
      }
      break;
    // We can have '<', '<=', or '<>'
    case '<':
      token->type = lessym;
      cur = getc(input);
      if (cur == '>') {
        token->type = neqsym;
        token->val[tokenLength++] = cur;
      }
      else if (cur == '=') {
        token->type = leqsym;
        token->val[tokenLength++] = cur;
      }
      else {
        ungetc(cur, input);
      }
      break;
    case ',':
      token->type = commasym;
      break;
    case ';':
      token->type = semicolonsym;
      break;
    case '=':
      token->type = eqsym;
      break;
    case ':':
      cur = getc(input);
      token->val[tokenLength++] = cur;
      if (cur != '=') {
        fprintf(stderr, "ERROR: '=' expected after ':'.\n");
        exit(EXIT_FAILURE);
      }
      token->type = becomessym;
      break;
    case '.':
      token->type = periodsym;
      break;
    default:
      fprintf(stderr, "ERROR: Unknown character '%c'.\n", cur);
      exit(EXIT_FAILURE);
  }

  return token;
}

// Returns the `tokentype` that correctly
// represents the type of `val`.
// If it is not a reserved word, its type will be `identsym`.
// Otherwise it will be the type of the reserved word.
int parseWordType(char *val) {
  if (strcmp(val, "const") == 0) return constsym;
  if (strcmp(val, "var") == 0) return varsym;
  if (strcmp(val, "procedure") == 0) return procsym;
  if (strcmp(val, "begin") == 0) return beginsym;
  if (strcmp(val, "end") == 0) return endsym;
  if (strcmp(val, "if") == 0) return ifsym;
  if (strcmp(val, "then") == 0) return thensym;
  if (strcmp(val, "else") == 0) return elsesym;
  if (strcmp(val, "while") == 0) return whilesym;
  if (strcmp(val, "do") == 0) return dosym;
  if (strcmp(val, "read") == 0) return readsym;
  if (strcmp(val, "odd") == 0) return oddsym;
  if (strcmp(val, "call") == 0) return callsym;
  if (strcmp(val, "read") == 0) return readsym;
  if (strcmp(val, "write") == 0) return writesym;
  else return identsym;
}

// Prints our symbol table's contents to lexemetable.txt
void printLexemeTable(int numTokens, Token *tokens[]) {
  FILE *out = fopen(LEXEME_TABLE_PATH, "w");
  int i;

  // Print the header for our file
  fprintf(out, "lexeme\t\ttoken type\n");

  // Print out all of our tokens, separated by a tab,
  // to lexemetable.txt
  for (i = 0; i < numTokens; i++) {
    fprintf(out, "%s\t\t%d\n", tokens[i]->val, tokens[i]->type);
  }
}

// Prints our symbol table's contents to lexemelist.txt
void printLexemeList(int numTokens, Token *tokens[]) {
  FILE *out = fopen(LEXEME_LIST_PATH, "w");
  int i;

  // Print all of our tokens, separated by a space,
  // and including the `val` if it is a num/ident
  // to lexemelist.txt
  fprintf(out, "%d", tokens[0]->type); 
  for (i = 1; i < numTokens; i++) {
    fprintf(out, " %d", tokens[i]->type);

    // If we're at an identifier or number, print out its value
    if (tokens[i]->type == identsym || tokens[i]->type == numbersym) {
      fprintf(out, " %s", tokens[i]->val);
    }
  }

  fclose(out);
}

// Reads in the contents of the raw input file, input.txt,
// removes comments, and writes the contents without comments
// into clean_input.txt
// Raises an error if there are any unfinished comments in the file.
void cleanRawFile() {
  FILE *rawInput = fopen(RAW_INPUT_PATH, "r");
  FILE *out = fopen(CLEAN_INPUT_PATH, "w");
  char contents[MAX_TOKENS * (MAX_IDENT_LEN + 1)];

  // Read the whole file into our contents array
  int fileSize;
  while (!feof(rawInput)) {
    contents[fileSize++] = getc(rawInput);
  }

  // Loop through our contents array and store
  // everything that is not a comment
  int i;
  for (i = 0; i < fileSize - 1; i++) {

    // Detect and skip comments
    if (contents[i] == '/') {
      if (contents[i+1] == '*') {

        // Skip the /* part of the comment
        i += 2;
        // Continue until we find the end of the comment.
        while (!(contents[i] == '*' && contents[i+1] == '/')) {
          i++;

          // If the file ends before the comment does,
          // exit and alert the user
          if (i == fileSize) {
            fprintf(stderr, "ERROR: EOF reached before comment termination.\n");
            exit(EXIT_FAILURE);
          }
        }

        // Skip the */ part of the comment
        i += 2;
      }
    }

    // Print the current character to our output file
    fprintf(out, "%c", contents[i]);
  }

  fclose(rawInput);
  fclose(out);
}
