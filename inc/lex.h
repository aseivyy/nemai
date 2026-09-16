#ifndef LEX
#define LEX

typedef enum TokenType_e { ret, add, sub, mult, divide, ca, df, number, av, ga, aa, ase, rse, asea, rsea, string, obracket_block, cbracket_block, obracket_sub, cbracket_sub, dv, ds, sp, word, eof, error } TokenType;

typedef struct {
	TokenType token;
	void *data;
} Token;

extern int lexLine;

Token Lex();

#endif
