#include "lexer.h"
#include "parser.h"
// TODO: #include "vm.h"

int main() {
  generateLexemes();
  parse();
  // TODO: runProgram();
}
