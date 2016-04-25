#ifndef STRUCTS_H
#define STRUCTS_H

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
    readsym , elsesym, unknownsym
} token_type;

typedef enum {
    consttype = 1, vartype, proctype
} symbol_type;

typedef struct {
    int kind;      // const = 1, var = 2, proc = 3
    char name[MAX_IDENT_LEN+1]; // name up to 11 chars
    int val;       // number (ASCII value)
    int level;     // L level
    int addr;      // M address
    char procIdent[MAX_IDENT_LEN+1]; // name of the procedure declared in
} Symbol;

// Represents each line of input
typedef struct {
    int op; // opcode
    int l; // lexicographical level
    int m; // a number, program address, data address, or ID for OPR 2
} Instruction;

typedef enum {
    LIT = 1, OPR, LOD, STO, CAL, INC, JMP, JPC, SIO
} op_code;

typedef enum {
    RTN = 0, NEG, ADD, SUB, MUL, DIV, ODD, MOD, EQL, NEQ, LSS, LEQ, GTR, GEQ
} m_code;

#endif
