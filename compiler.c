#include "lexer.c"
#include "parser.c"
#include "vm.c"

int main() {
  generateLexemes();
  parse();
  runProgram();
}
