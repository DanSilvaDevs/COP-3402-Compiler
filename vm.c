// Written by:	Tuan Pham
// Date: 		2/7/16
// Purpose:		PL0 Virtual Machine
// Input: 		Text file located in the same directory named "mcode.txt"
// Output:		Text file located in the same directory named "stacktrace.txt"
// Process:		First loads instructions from mcode.txt into an instruction array called code[CODE_LENGTH]
//				Then properly simulates PL0 virtual machine stepping through the code and executing it.  
// NOTE:		Must have a halt instruction to work properly. 

#include <stdio.h>
#define MAX_STACK_HEIGHT 200
#define CODE_LENGTH 500
#define MAX_LEXI_LEVELS 3

typedef struct {
	int op;
	int l;
	int m;
}Instruction;
const char* isa[] =  {
	"",
	"LIT",
	"OPR",
	"LOD",
	"STO",
	"CAL",
	"INC",
	"JMP",
	"JPC",
	"SIO",
	"SIO",
	"SIO"
};
int SP = 0;
int BP = 1;
int PC = 0;
int stack[MAX_STACK_HEIGHT];


void printStack(FILE* outfile){
	int i;
	for(i=1; i<=SP; i++) {
		if (BP != 1 && i == BP) fprintf(outfile, "| ");
		fprintf(outfile, "%d ", stack[i]);
	}
	fprintf(outfile, "\n");
}

int base(int l, int base) {
	int b1 = base;
	while (l > 0) {
		b1 = stack[b1 + 1];
		l--;
	}
	return b1;
}
void operation(Instruction IR, FILE* outfile){
	switch(IR.m) {
		case 0:		//ret
			SP = BP-1;
			PC = stack[SP + 4];
			BP = stack[SP + 3];
			break;
		case 1:		//neg
			stack[SP] = -stack[SP];
			break;
		case 2:		//add
			SP = SP - 1;
			stack[SP] = stack[SP] + stack[SP + 1];
			PC = PC + 1;
			break;
		case 3:		//sub
			SP = SP - 1;
			stack[SP] = stack[SP] - stack[SP + 1];
			PC = PC + 1;
			break;
		case 4:		//mul
			SP = SP - 1;
			stack[SP] = stack[SP] * stack[SP + 1];
			PC = PC + 1;
			break;
		case 5:		//div
			SP = SP - 1;
			stack[SP] = stack[SP] / stack[SP + 1];
			PC = PC + 1;
			break;
		case 6:		//odd
			stack[SP] = stack[SP] % 2;
			PC = PC + 1;
			break;
		case 7:		//mod
			SP = SP - 1;
			stack[SP] = stack[SP] % stack[SP + 1];
			PC = PC + 1;
			break;
		case 8:		//eql
			SP = SP - 1;
			if (stack[SP] == stack[SP + 1]) stack[SP] = 1;
			else {
				stack[SP] = 0;
			}
			PC = PC + 1;
			break;
		case 9:		//neq
			SP = SP - 1;
			if (stack[SP] != stack[SP + 1]) stack[SP] = 1;
			else {
				stack[SP] = 0;
			}
			PC = PC + 1;
			break;
		case 10:	//lss
			SP = SP - 1;
			if (stack[SP] < stack[SP + 1]) stack[SP] = 1;
			else {
				stack[SP] = 0;
			}
			PC = PC + 1;
			break;
		case 11:	//leq
			SP = SP - 1;
			if (stack[SP] <= stack[SP + 1]) stack[SP] = 1;
			else {
				stack[SP] = 0;
			}
			PC = PC + 1;
			break;
		case 12:	//gtr
			SP = SP - 1;
			if (stack[SP] > stack[SP + 1]) stack[SP] = 1;
			else {
				stack[SP] = 0;
			}
			PC = PC + 1;
			break;
		case 13:	//geq
			SP = SP - 1;
			if (stack[SP] >= stack[SP + 1]) stack[SP] = 1;
			else {
				stack[SP] = 0;
			}
			PC = PC + 1;
			break;
	}
}
int decode(Instruction IR, FILE* outfile){
	fprintf(outfile, "%3d%4s%4d%4d", PC, isa[IR.op], IR.l, IR.m);

	switch (IR.op) {
		//LIT Case
		case 1:
			SP = SP + 1;
			stack[SP] = IR.m;
			PC = PC + 1;
			break;
		//OPR Case
		case 2:
			operation(IR, outfile);
			break;
		//LOD Case
		case 3:
			SP = SP + 1;
			stack[SP] = stack[base(IR.l, BP) + IR.m];
			PC = PC + 1;
			break;
		//STO Case
		case 4:
			stack[base(IR.l, BP) + IR.m] = stack[SP];
			SP = SP - 1;
			PC = PC + 1;
			break;
		//CAL Case
		case 5:
			stack[SP+1] = 0;
			stack[SP+2] = base(IR.l, BP);
			stack[SP+3] = BP;
			stack[SP+4] = PC+1;
			BP = SP+1;
			PC = IR.m;
			break;
		//INC Case
		case 6:
			SP += IR.m;
			PC = PC + 1;
			break;
		//JMP CASE
		case 7:
			PC = IR.m;
			break;
		//JPC Case
		case 8:
			if (stack[SP] == 0) PC = IR.m;
			else {
				PC = PC + 1;
			}
			SP = SP - 1;
			break;
		//SIO Case (print)
		case 9:
			printf("%d\n", stack[SP]);
			SP--;
			PC = PC + 1;
			break;
		//SIO Case (read)
		case 10:
			SP++;
			scanf("%d", &stack[SP]);
			PC = PC + 1;
			break;
		//SIO Case (halt)
		case 11:
			return 1;
	}
	fprintf(outfile, "%4d%4d%4d\t", PC, BP, SP);
	printStack(outfile);
	return 0;
}

void main() {
	Instruction IR;
	Instruction code[CODE_LENGTH];
	FILE* infile = fopen("mcode.txt", "r");
	FILE* outfile = fopen("stacktrace.txt", "w");

	//load instructions into code array
	int i = 0;
	while (	fscanf(infile, "%d %d %d", &code[i].op, &code[i].l, &code[i].m) != EOF) {
		fprintf(outfile, "%d\t%s\t%d\t%d\n", i, isa[code[i].op], code[i].l, code[i].m);
		i++;
	}
	fclose(infile);

	//Fetches and decodes instructions until halt instruction
	do{
		IR = code[PC];
	}while(decode(IR, outfile) == 0);
	fprintf(outfile, "\nSuccessfully halted.");
    fclose(outfile);
}

