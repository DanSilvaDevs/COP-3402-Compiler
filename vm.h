#ifndef VM_H
#define VM_H

#include <stdio.h>
#include "structs.h"
#include "constants.h"

void printStack(FILE* outfile);
int base(int l, int base);
void operation(Instruction IR, FILE* outfile);
int decode(Instruction IR, FILE* outfile);
void runProgram();

#endif
