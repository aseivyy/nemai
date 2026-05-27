#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

enum tokens { error, eof, equal, isequal, isnequal, greater, greater_equal, lesser, lesser_equal, obracket_block, cbracket_block, obracket_sub, cbracket_sub, add, sub, mult, divide, number, word, kif, kfi, ret, df, dv };
enum nodetypes { nroot, ndf, nret, nnum };
enum types { tvar };

enum ASM_FIRST { ADR, RET };

typedef char STATUS;

typedef struct {
	enum tokens token;
	void *symtable;
} TOKEN;

typedef struct typelist_s {
	char* name;
	char size;
	struct typelist_s *next;
} typelist;

typedef struct parsetable_s {
	struct parsetable_s **things;
	enum nodetypes type;
	char state;
	void* args;
} parsetable;

typedef struct symtableEntry_s {
	char* name;
	typelist *type;
	enum types category;
	struct symtableEntry_s *next;
} symtableEntry;

typedef struct symtable_s {
	symtableEntry *things;
	struct symtable_s **children;
	int nthings;
} symtable;

typedef struct functionTable_s {
	char *name;
	typelist *type;
	struct functionTable_s *next;
} functionTable;

typedef struct {
	uint16_t Machine;
	uint16_t nSections;
	uint32_t Time;
	uint32_t oSymTable;
	uint32_t nSymbols;
	uint16_t sOptHeader;
	uint16_t Flags;
} CoffHeader;

typedef struct {
	uint8_t Name[8];
	uint32_t sVirt;
	uint32_t oVirt;
	uint32_t sRaw;
	uint32_t oRaw;
	uint32_t oRelocations;
	uint32_t oLineNumbers;
	uint16_t nRelocations;
	uint16_t nLineNumbers;
	uint32_t Flags;
} CoffSectionHeader;

typedef struct {
	uint32_t Zero;
	uint32_t oName;
	uint32_t osValue;
	uint16_t xSection;
	uint16_t Type;
	uint8_t Class;
	uint8_t nAux;
} __attribute__((packed)) CoffSymbolEntry;

typelist *Typelist;
int nTypes;

parsetable *ParseRoot;

symtable *SymtableRoot;
symtable* *SymtableFamily;

functionTable *FunctionRoot;

int lexLine = 1;

parsetable* *ParseFamily;
enum nodetypes *ParseFamilyType;
int ParseFamilyMembers = 0;
int *ParseFamilyFilled;

/* Asm generation */
int asmLines = 0;
enum ASM_FIRST *asmFirst;
char **asmSecond;
char **asmThird;

CoffHeader coffHeader;
CoffSectionHeader coffSectionHeader;
CoffSymbolEntry *coffSymbolEntries;
char *coffString;

uint32_t textSize = 0;
int nSymbols = 0;
int osText = 0;
int sString = 0;

const char BIN_RET = 0xC3;
const char REX_W = 0x48;
const char MOV_IMM_REG = 0xB8;
const char REG_AX = 0x0;

#define ERROR 1
#define SUCCESS 0

FILE *file;
FILE *out;

char isNum(char c) {
	if (c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7' || c == '8' || c == '9') return 1;
	return 0;
}

int64_t charToNum(char c) {
	return c - '0';
}

char isLetter(char c) {
	if ((c >= 'a' && c <= 'z') || (c >= 'A' &&  c <= 'Z') || c == '_' || c == '^') return 1;
	return 0;
}

enum tokens isKeyword(char *s) {
	if (strcmp(s, "if") == 0) return kif;
	if (strcmp(s, "fi") == 0) return kfi;
	if (strcmp(s, "ret") == 0) return ret;
	if (strcmp(s, "df") == 0) return df;
	if (strcmp(s, "dv") == 0) return dv;
	return word;
}

TOKEN Lex() {
	char c;
	do {
		c = fgetc(file);
		if (c == '\n') lexLine++;
	} while (c == ' ' || c == '\n' || c == '\t' || c == ';');

	if (c == EOF) {
		TOKEN token;
		token.token = eof;
		token.symtable = (void*) 0;
		return token;
	}
	
	if (c == '=') {
		char next = fgetc(file);
		if (next == '=') {
			TOKEN token;
			token.token = isequal;
			token.symtable = (void*) 0;
			return token;
		} else {
			fseek(file, -1, SEEK_CUR);

			TOKEN token;
			token.token = equal;
			token.symtable = (void*) 0;
			return token;
		}
	}

	if (c == '!') {
		char next = fgetc(file);
		if (next == '=') {
			TOKEN token;
			token.token = isnequal;
			token.symtable = (void*) 0;
			return token;
		} else {
			TOKEN token;
			token.token = error;
			token.symtable = (void*) 0;
			return token;
		}
	}

	if (c == '>') {
		char next = fgetc(file);
		if (next == '=') {
			TOKEN token;
			token.token = greater_equal;
			token.symtable = (void*) 0;
			return token;
		} else {
			fseek(file, -1, SEEK_CUR);

			TOKEN token;
			token.token = greater;
			token.symtable = (void*) 0;
			return token;
		}
	}

	if (c == '<') {
		char next = fgetc(file);
		if (next == '=') {
			TOKEN token;
			token.token = lesser_equal;
			token.symtable = (void*) 0;
			return token;
		} else {
			fseek(file, -1, SEEK_CUR);

			TOKEN token;
			token.token = lesser;
			token.symtable = (void*) 0;
			return token;
		}
	}

	if (c == '(') {
		TOKEN token;
		token.token = obracket_block;
		token.symtable = (void*) 0;
		return token;
	}

	if (c == ')') {
		TOKEN token;
		token.token = cbracket_block;
		token.symtable = (void*) 0;
		return token;
	}

	if (c == '[') {
		TOKEN token;
		token.token = obracket_sub;
		token.symtable = (void*) 0;
		return token;
	}

	if (c == ']') {
		TOKEN token;
		token.token = cbracket_sub;
		token.symtable = (void*) 0;
		return token;
	}

	if (c == '+') {
		TOKEN token;
		token.token = add;
		token.symtable = (void*) 0;
		return token;
	}

	if (c == '-') {
		TOKEN token;
		token.token = sub;
		token.symtable = (void*) 0;
		return token;
	}

	if (c == '*') {
		TOKEN token;
		token.token = mult;
		token.symtable = (void*) 0;
		return token;
	}

	if (c == '/') {
		TOKEN token;
		token.token = divide;
		token.symtable = (void*) 0;
		return token;
	}

	if (isNum(c) == 1) {
		int64_t num = charToNum(c);
		char next = fgetc(file);
		
		while (isNum(next) == 1) {
			num *= 10;
			num += charToNum(next);
			next = fgetc(file);
		}
		fseek(file, -1, SEEK_CUR);

		TOKEN token;
		token.token = number;
		token.symtable = (int64_t*) malloc(sizeof(int64_t));
		*((int64_t*) token.symtable) = num;
		return token;
	}

	if (isLetter(c) == 1) {
		char *read = (char*) malloc(sizeof(char));
		read[0] = c;
		int letters = 1;
		
		c = fgetc(file);

		while (isLetter(c) == 1 || isNum(c) == 1) {
			letters++;
			read = (char*) realloc(read, letters);
			read[letters - 1] = c;

			c = fgetc(file);
		}

		letters++;
		read = (char*) realloc(read, letters);
		read[letters - 1] = '\0';

		fseek(file, -1, SEEK_CUR);

		enum tokens check = isKeyword(read);
		if (check == word) {
			TOKEN token;
			token.token = word;
			token.symtable = read;
			return token;
		} else {
			free(read);
			TOKEN token;
			token.token = check;
			token.symtable = (void*) 0;
			return token;
		}
	}
	
	printf("Char not able to process: %c", c);

	TOKEN token;
	token.token = error;
	return token;
}

void BaseTypeSetup() {
	Typelist = (typelist*) malloc(sizeof(typelist));
	nTypes = 5;
	Typelist->name = (char*) malloc(sizeof(char) * 3);
	strcpy(Typelist->name, "^0");
	Typelist->size = 0;

	typelist *b8 = (typelist*) malloc(sizeof(typelist));
	b8->name = (char*) malloc(sizeof(char) * 3);
	strcpy(b8->name, "^8");
	b8->size = 8;
	Typelist->next = b8;

	typelist *b16 = (typelist*) malloc(sizeof(typelist));
	b16->name = (char*) malloc(sizeof(char) * 4);
	strcpy(b16->name, "^16");
	b16->size = 16;
	Typelist->next->next = b16;

	typelist *b32 = (typelist*) malloc(sizeof(typelist));
	b32->name = (char*) malloc(sizeof(char) * 4);
	strcpy(b32->name, "^32");
	b32->size = 32;
	Typelist->next->next->next = b32;

	typelist *b64 = (typelist*) malloc(sizeof(typelist));
	b64->name = (char*) malloc(sizeof(char) * 4);
	strcpy(b64->name, "^64");
	b64->size = 64;
	Typelist->next->next->next->next = b64;
}

STATUS addVar(symtable **pcurNode, TOKEN *type, TOKEN *name) {
	symtable *curNode = *(pcurNode);
	typelist *curTypelist = Typelist;
	
	symtableEntry **lastEntry = &(curNode->things);
	for (int i = 0; i < curNode->nthings; i++) {
		lastEntry = &((*lastEntry)->next);
	}
	*lastEntry = (symtableEntry*) malloc(sizeof(symtableEntry));
	
	if (type->token != word) {
		printf("nemai:Parse \tVariable type isn't a type on line %d", lexLine);
		printf("%c", '\n');
		return ERROR;
	}
		
	int isAType = 0;
	
	for (int i = 0; i < nTypes; i++) {
		if (strcmp(type->symtable, curTypelist->name) == 0) {
			i = nTypes;
			isAType++;
		} else {
			curTypelist = curTypelist->next;
		}
	}
	if (isAType < 1) {
		printf("nemai:Parse \tVariable type isn't a registered type on line %d", lexLine);
		printf("%c", '\n');
		return ERROR;
	}
		
	(*lastEntry)->type = curTypelist;
		
	if (name->token != word) {
		printf("nemai:Parse \tVariable name is reserved or forbidden on line %d", lexLine);
		printf("%c", '\n');
		return ERROR;
	}
		
	int i = 0;
	int nameCheckFail = 0;
	(*lastEntry)->name = malloc(sizeof(char));
		
	while((SymtableFamily[i] != (void*) 0) && nameCheckFail == 0) {
		int nodenthings = SymtableFamily[i]->nthings;
		symtableEntry *curListEntry = SymtableFamily[i]->things;
		for (int i = 0; i < nodenthings && nameCheckFail == 0; i++) {
			if(strcmp(curListEntry->name, name->symtable) == 0) {
				nameCheckFail++;
			}
			curListEntry = curListEntry->next;
		}
		i++;
	}
	
	if (nameCheckFail > 0) {
		printf("nemai:Parse \tAttempt to redefine a variable on line %d", lexLine);
		printf("%c", '\n');
		return ERROR;
	}
		
	(*lastEntry)->name[0] = ((char*) (name->symtable))[0];
	int nchars;
	for (int i = 1; ((char*) (name->symtable))[i] != '\0'; i++) {
		(*lastEntry)->name = realloc((*lastEntry)->name, sizeof(char) * (i + 1));
		(*lastEntry)->name[i] = ((char*) (name->symtable))[i];
		nchars = i;
	}
		
	(*lastEntry)->name = realloc((*lastEntry)->name, sizeof(char) * (nchars + 2));
	(*lastEntry)->name[nchars + 1] = '\0';

	(*lastEntry)->category = tvar;
				
	(curNode->nthings)++;
	return SUCCESS;
}

STATUS Parse() {
	BaseTypeSetup();
	
	typelist *curTypelist = Typelist;

	/* Uncomment to print all registered types */
	/* printf("\n"); */
	/* for (int i = 0; i < nTypes; i++) { */
	/* 	printf("%s with size %d,  ", curTypelist->name, curTypelist->size); */
	/* 	curTypelist = curTypelist->next; */
	/* } */
	/* printf("\n\n"); */

	ParseRoot = (parsetable*) malloc(sizeof(parsetable));
	parsetable *curParseNode = ParseRoot;
	ParseRoot->state = 1;
	ParseRoot->things = (parsetable**) malloc(sizeof(parsetable*));
	ParseRoot->things[0] = (parsetable*) malloc(sizeof(parsetable));
	ParseRoot->things[0]->state = 0;
	ParseRoot->type = nroot;

	ParseFamily = (parsetable**) malloc(sizeof(parsetable*) * 2);
	ParseFamily[0] = ParseRoot;
	ParseFamily[1] = (void*) 0;
	
	/* typedef struct parsetable_s { */
	/* 	struct parsetable_s **things; */
	/* 	enum types nodetype; */
	/* 	void* args; */
	/* } parsetable; */

	SymtableRoot = (symtable*) malloc(sizeof(symtable));
	SymtableRoot->nthings = 0;
	symtable *curNode = SymtableRoot;
	SymtableRoot->children = (symtable**) malloc(sizeof(symtable*));
	*(SymtableRoot->children) = (void*) 0;

	SymtableFamily = (symtable**) malloc(sizeof(symtable*) * 2);
	SymtableFamily[0] = SymtableRoot;
	SymtableFamily[1] = (void*) 0;

	// I know a waste of memory I couldnt think of any better way, so I guess first entry will be just always set to empty for now
	FunctionRoot = (functionTable*) malloc(sizeof(functionTable));
	FunctionRoot->next = (void*) 0;

	
	for (TOKEN i = Lex(); i.token != eof && i.token != error; i = Lex()) {
		if (i.token == obracket_block) {
			TOKEN node = Lex();
			if (node.token == dv) {
				TOKEN type = Lex();
				TOKEN name = Lex();
				if (type.token == eof || name.token == eof) {
					printf("nemai:Parse \tFile ended on unfinished variable definition on line %d", lexLine);
					printf("%c", '\n');
					return ERROR;
				}

				if (type.token != word || name.token != word) {
					printf("nemai:Parse \tWrong parameters given to function \"dv\" or given less than required 2 on line %d", lexLine);
					printf("%c", '\n');
					return ERROR;
				}
				
				if (addVar(&curNode, &type, &name) == ERROR) return ERROR;

				TOKEN ending = Lex();
				if (ending.token == eof) {
					printf("nemai:Parse \tFile ended on unfinished variable definition on line %d", lexLine);
					printf("%c", '\n');
					return ERROR;
				} else if (ending.token != cbracket_block) {
					printf("nemai:Parse \tFunction \"df\" can only take 2 arguments, but given more on line %d", lexLine);
					printf("%c", '\n');
					return ERROR;
				}
			} else if (node.token == df) {
				if (curNode != SymtableRoot) {
					printf("nemai:Parse \tAttempt to define a function inside of a function on line %d", lexLine);
					printf("%c", '\n');
					return ERROR;
				}

				int parselast = 0;
				for (int i = 0; ParseRoot->things[i]->state != 0; i++) {
					parselast = i + 1;
				}

				curParseNode = ParseRoot->things[parselast];
				
				ParseRoot->things = (parsetable**) realloc(ParseRoot->things, sizeof(parsetable*) * (parselast + 2));
				ParseRoot->things[parselast + 1] = (parsetable*) malloc(sizeof(parsetable));
				ParseRoot->things[parselast + 1]->state = 0;
				
				ParseRoot->things[parselast]->state = 1;
				ParseRoot->things[parselast]->things = (parsetable**) malloc(sizeof(parsetable*));
				ParseRoot->things[parselast]->things[0] = (parsetable*) malloc(sizeof(parsetable));
				ParseRoot->things[parselast]->things[0]->state = 0;
				ParseRoot->things[parselast]->type = ndf;
				ParseRoot->things[parselast]->args = (functionTable**) malloc(sizeof(functionTable*));
				
				ParseFamily = (parsetable**) realloc(ParseFamily, sizeof(parsetable*) * 3);
				ParseFamily[1] = curParseNode;
				ParseFamily[2] = (void*) 0;


				functionTable **lastNode = &(FunctionRoot);
				while ((*lastNode)->next != (void*) 0) {
					lastNode = &((*lastNode)->next);
				}
				lastNode = &((*lastNode)->next);
				
				(*lastNode) = (functionTable*) malloc(sizeof(functionTable));
				(*lastNode)->next = (void*) 0;

				((functionTable**) (ParseRoot->things[parselast]->args))[0] = (*lastNode);
				
				TOKEN fType = Lex();
				if (fType.token != word) {
					printf("nemai:Parse \tFunction type isn't a type on line %d", lexLine);
					printf("%c", '\n');
					return ERROR;
				}

				curTypelist = Typelist;
				int isAType = 0;
				
				for (int i = 0; i < nTypes; i++) {
					if (strcmp(fType.symtable, curTypelist->name) == 0) {
						i = nTypes;
						isAType++;
					} else {
						curTypelist = curTypelist->next;
					}
				}
				
				if (isAType < 1) {
					printf("nemai:Parse \tFunction type isn't a registered type on line %d", lexLine);
					printf("%c", '\n');
					return ERROR;
				}

				(*lastNode)->type = curTypelist;

				TOKEN fName = Lex();
				if (fName.token != word) {
					printf("nemai:Parse \tFunction name is reserved or forbidden on line %d", lexLine);
					printf("%c", '\n');
					return ERROR;
				}

				int nameCheckFail = 0;
				(*lastNode)->name = malloc(sizeof(char));

				functionTable **nametester = &(FunctionRoot->next);
				while(((*nametester)->next != (void*) 0) && ((*nametester) != (void*) 0) && nameCheckFail == 0) {
					if (strcmp((*nametester)->name, fName.symtable) == 0) nameCheckFail++;
					nametester = &((*nametester)->next);
				}
					

				if (nameCheckFail > 0) {
					printf("nemai:Parse \tAttempt to redefine a function on line %d", lexLine);
					printf("%c", '\n');
					return ERROR;
				}

				(*lastNode)->name[0] = ((char*) (fName.symtable))[0];
				int nchars;
				for (int i = 1; ((char*) (fName.symtable))[i] != '\0'; i++) {
					(*lastNode)->name = realloc((*lastNode)->name, sizeof(char) * (i + 1));
					(*lastNode)->name[i] = ((char*) (fName.symtable))[i];
					nchars = i;
				}
				
				(*lastNode)->name = realloc((*lastNode)->name, sizeof(char) * (nchars + 2));
				(*lastNode)->name[nchars + 1] = '\0';

				symtable **symtableChildChecker = SymtableRoot->children;
				int toPlace = 0;
				for (int i = 0; symtableChildChecker[i] != (void*) 0; i++) {
					toPlace = i + 1;
				}

				symtable *NewSymtable = (symtable*) malloc(sizeof(symtable));
				NewSymtable->nthings = 0;
				
				symtableChildChecker[toPlace+1] = (void*) 0;

				NewSymtable->children = (symtable**) malloc(sizeof(symtable*));
				*(NewSymtable->children) = (void*) 0;

				symtableChildChecker[toPlace] = NewSymtable;

				SymtableFamily = (symtable**) realloc(SymtableFamily, sizeof(symtable*) * 3);
				SymtableFamily[2] = (void*) 0;
				SymtableFamily[1] = NewSymtable;

				curNode = NewSymtable;

				TOKEN bracketCheck = Lex();
				if (bracketCheck.token != obracket_sub) {
					printf("nemai:Parse \tMissing parameter list in function definition on line %d", lexLine);
					printf("%c", '\n');
					return ERROR;
				}
				
				TOKEN paramType = Lex();

				if (paramType.token != cbracket_sub) {
					TOKEN paramName = Lex();
					while (paramType.token == word && paramName.token == word) {
						if (addVar(&curNode, &paramType, &paramName) == ERROR) return ERROR;
						paramType = Lex();
						if (paramType.token != cbracket_sub) paramName = Lex();
					}
				}
			} else if (node.token == ret) {
				int parselast = 0;
				for (int i = 0; curParseNode->things[i]->state != 0; i++) {
					parselast = i + 1;
				}

				curParseNode->things = (parsetable**) realloc(curParseNode->things, sizeof(parsetable*) * (parselast + 2));
				curParseNode->things[parselast + 1] = (parsetable*) malloc(sizeof(parsetable));
				curParseNode->things[parselast + 1]->state = 0;

				curParseNode = curParseNode->things[parselast];
				curParseNode->state = 1;
				curParseNode->things = (void*) 0;
				curParseNode->type = nret;
				curParseNode->things = (parsetable**) malloc(sizeof(parsetable*));
				curParseNode->things[0] = (parsetable*) malloc(sizeof(parsetable));
				curParseNode->things[0]->state = 0;

				int ParseFamilyMembers = 0;
				while (ParseFamily[ParseFamilyMembers] != (void*) 0) {
					ParseFamilyMembers++;
				}

				ParseFamily = (parsetable**) realloc(ParseFamily, sizeof(parsetable*) * (ParseFamilyMembers + 2));
				ParseFamily[ParseFamilyMembers] = curParseNode;
				ParseFamily[ParseFamilyMembers + 1] = (void*) 0;
				
			} else if (node.token <= word) {
				// add support for user made functios later and dont forget it
				printf("nemai:Parse \tAttempt to call an undefined function on line %d", lexLine);
				printf("%c", '\n');
				return ERROR;
			}

		} else if (i.token == number) {
			if (curParseNode->type == nret) {
				if (curParseNode->things[0]->state != 0) {
					printf("nemai:Parse \tAttempt to return more than one value on line %d", lexLine);
					printf("%c", '\n');
					return ERROR;
				}
				
				curParseNode->things[0]->state = 1;
				curParseNode->things[0]->type = nnum;
				curParseNode->things[0]->args = (int64_t*) malloc(sizeof(int64_t));
				((int64_t*) (curParseNode->things[0]->args))[0] = *((int64_t*) i.symtable);
			} else {
				printf("nemai:Parse \tAttempt to use a mumber in unknown way on line %d", lexLine);
				printf("%c", '\n');
				return ERROR;
			}
		} else if (i.token == cbracket_block) {
			if (curParseNode->type != nret) {
				int SymtableFamilyMembers = 0;
				while (SymtableFamily[SymtableFamilyMembers] != (void*) 0) {
					SymtableFamilyMembers++;
				}
				curNode = SymtableFamily[SymtableFamilyMembers - 2];
				SymtableFamily = (symtable**) realloc(SymtableFamily, sizeof(symtable*) * (SymtableFamilyMembers - 1));
				SymtableFamily[SymtableFamilyMembers - 1] = (void*) 0;
			}
				
			int ParseFamilyMembers = 0;
			while (ParseFamily[ParseFamilyMembers] != (void*) 0) {
				ParseFamilyMembers++;
			}
			curParseNode = ParseFamily[ParseFamilyMembers - 2];
			ParseFamily = (parsetable**) realloc(ParseFamily, sizeof(parsetable*) * (ParseFamilyMembers - 1));
			ParseFamily[ParseFamilyMembers - 1] = (void*) 0;
		}
	}
	return SUCCESS;
}

STATUS LexSetup(char *filename) {
	file = fopen(filename, "r");
	lexLine = 1;
	
	if (file == NULL) {
		printf("nemai:LexSetup \tFile doesn't exist or couldn't be opened");
		printf("%c", '\n');
		return ERROR;
	}

	char c = fgetc(file);
	if (c == EOF) {
		printf("nemai:LexSetup \tFile is empty");
		printf("%c", '\n');
		return ERROR;
	}

	fseek(file, -1, SEEK_CUR);
	
	return SUCCESS;
}

void ReadParseTree(parsetable *table) {
	for (int i = 0; table->things[i]->state != 0; i++) {
		switch(table->things[i]->type) {
		case nroot:
			break;
		case ndf:
			printf("Function node with name \"%s\"\n", ((functionTable**) (table->things[i]->args))[0]->name);
			ReadParseTree(table->things[i]);
			printf("Function node end\n\n");
			break;
		case nret:
			printf("Return node\n");
			break;
		default:
			printf("idk something ig %d\n", table->things[i]->type);
			break;
		}
	}
}

STATUS GenAsm(parsetable *table) {
	for (int i = 0; table->things[i]->state != 0; i++) {
		switch(table->things[i]->type) {
		case nroot:
			break;
		case ndf:
			/* On function start */
			asmFirst = realloc(asmFirst, (sizeof(enum ASM_FIRST)) * (asmLines + 1));
			asmSecond = realloc(asmSecond, sizeof(char*) * (asmLines + 1));
			asmThird = realloc(asmThird, sizeof(char*) * (asmLines + 1));
			
			asmFirst[asmLines] = ADR;
			asmSecond[asmLines] = ((functionTable**) (table->things[i]->args))[0]->name;
			asmThird[asmLines] = (void*) 0;

			asmLines++;
			nSymbols++;

			coffSymbolEntries = realloc(coffSymbolEntries, sizeof(CoffSymbolEntry) * nSymbols);
			coffSymbolEntries[nSymbols - 1].oName = sString;
			coffSymbolEntries[nSymbols - 1].Zero = 0;
			coffSymbolEntries[nSymbols - 1].osValue = osText;
			coffSymbolEntries[nSymbols - 1].xSection = 1;
			coffSymbolEntries[nSymbols - 1].Type = 0x20;
			coffSymbolEntries[nSymbols - 1].Class = 2;
			coffSymbolEntries[nSymbols - 1].nAux = 0;

			coffString = realloc(coffString, sString + strlen(((functionTable**) (table->things[i]->args))[0]->name) + 1);
			strcpy(coffString + sString, ((functionTable**) (table->things[i]->args))[0]->name);
			sString += strlen(((functionTable**) (table->things[i]->args))[0]->name) + 1;

			GenAsm(table->things[i]);
			/* On function end */
			break;
		case nret:
			if (table->things[i]->things[0]->state == 0) {
				asmFirst = realloc(asmFirst, (sizeof(enum ASM_FIRST)) * (asmLines + 1));
				asmSecond = realloc(asmSecond, sizeof(char*) * (asmLines + 1));
				asmThird = realloc(asmThird, sizeof(char*) * (asmLines + 1));
			
				asmFirst[asmLines] = RET;
				asmSecond[asmLines] = (void*) 0;
				asmThird[asmLines] = (void*) 0;

				fwrite(&BIN_RET, 1, 1, out);

				textSize++;
				asmLines++;
				osText++;
			} else {
				asmFirst = realloc(asmFirst, (sizeof(enum ASM_FIRST)) * (asmLines + 2));
				asmSecond = realloc(asmSecond, sizeof(char*) * (asmLines + 2));
				asmThird = realloc(asmThird, sizeof(char*) * (asmLines + 2));
			
				asmFirst[asmLines] = RET;
				asmSecond[asmLines] = (void*) 0;
				asmThird[asmLines] = (void*) 0;

				char opcode = MOV_IMM_REG + REG_AX;
				int64_t retValue = ((int64_t*) (table->things[i]->things[0]->args))[0];
				fwrite(&REX_W, 1, 1, out);
				fwrite(&opcode, 1, 1, out);
				fwrite(&retValue, 8, 1, out);
				
				fwrite(&BIN_RET, 1, 1, out);

				textSize += 11;
				asmLines++;
				osText += 11;
			}
			
			break;
		default:
			break;
		}
	}
	return SUCCESS;
}

void ReadAsm() {
	printf("\n-----------------\n");
	printf("|      ASM      |");
	printf("\n-----------------\n");

	for(int i = 0; i < asmLines; i++) {
	        switch(asmFirst[i]) {
		case ADR:
			printf("ADR %s\n", asmSecond[i]);
			break;
		case RET:
			printf("RET\n");
			break;
		default:
			break;
		}
	}
}

STATUS AsmWriteSetup(char *name) {
	char *outName;
	outName = (char*) malloc(strlen(name) + strlen(".obj") + 1);
	strcpy(outName, name);
	strcat(outName, ".obj");
	
	out = fopen(outName, "wb");
	
	if (out == NULL) {
		printf("nemai:AsmWrite \tCouldn't write to a file");
		printf("%c", '\n');
		return ERROR;
	}

	coffHeader.Machine = 0x8664;
	coffHeader.nSections = 1;
	coffHeader.Time = 0;
	coffHeader.oSymTable = 0;
	coffHeader.nSymbols = 0;
	coffHeader.sOptHeader = 0;
	coffHeader.Flags = 0;

        strncpy((char*) &(coffSectionHeader.Name[0]), ".text\0\0\0", 8);
	coffSectionHeader.sVirt = 0;
	coffSectionHeader.oVirt = 0;
	coffSectionHeader.oRaw = sizeof(CoffSectionHeader) + sizeof(CoffHeader);
	coffSectionHeader.oRelocations = 0;
	coffSectionHeader.oLineNumbers = 0;
	coffSectionHeader.nRelocations = 0;
	coffSectionHeader.nLineNumbers = 0;
	coffSectionHeader.Flags = 0x00000020 | 0x20000000;

	fwrite(&coffHeader, sizeof(CoffHeader), 1, out);
	fwrite(&coffSectionHeader, sizeof(CoffSectionHeader), 1, out);
	
	return SUCCESS;
}

STATUS AsmMake(char *name) {
	coffSymbolEntries = (CoffSymbolEntry*) malloc(sizeof(CoffSymbolEntry));
	coffString = (char*) malloc(sizeof(char) * 4);
	sString = 4;
	
	if (AsmWriteSetup(name) == ERROR) {
		return ERROR;
	}

	if (GenAsm(ParseRoot) == ERROR) {
		return ERROR;
	}
		
	fseek(out, 36, SEEK_SET);
	fwrite(&textSize, 4, 1, out);
	fseek(out, 0, SEEK_END);

	for (int i = 0; i < nSymbols; i++) {
		fwrite(&(coffSymbolEntries[i]), sizeof(CoffSymbolEntry), 1, out);
	}

	int osSymTable = sizeof(CoffHeader) + sizeof(CoffSectionHeader) + textSize;
	fseek(out, 8, SEEK_SET);
	fwrite(&osSymTable, 4, 1, out);
	printf("\n%d ", osSymTable);
	
	fseek(out, 12, SEEK_SET);
	fwrite(&nSymbols, 4, 1, out);

	fseek(out, 0, SEEK_END);

	fwrite(&sString, 4, 1, out);
	fwrite(coffString + 4, sString - 4, 1, out);
	
	return SUCCESS;
}

int main(int argc, char **argv) {
	if (argc < 2) {
		printf("nemai:Start \tPlease specify a file.");
		printf("\n\tUse \"nemai filename\"");
		printf("%c", '\n');
		return ERROR;
	} else if (argc > 2) {
		printf("nemai:Start \tYou can only compile one file at once");
		printf("\n\tUse \"nemai filename\"");
		printf("%c", '\n');
		return ERROR;
	}

	if (LexSetup(argv[1]) == ERROR) {
		return ERROR;
	}

	/* Uncomment to print tokens */
	/* TOKEN test = Lex(); */
	/* while (test.token != eof && test.token != error) { */
	/* 	printf("%d ", test.token); */
	/* 	test = Lex(); */
	/* } */
	/* printf("%c", '\n'); */

	/* LexSetup(argv[1]); */
	if (Parse() == ERROR) {
		return ERROR;
	}

	/* Dont uncomment unless using the test.ni file */
	/* printf("\n[some kind of debugging things to prove that it works]\n"); */
	/* printf("\nType of variable: %s\nName of it: %s\nCategory of it: %d\n", SymtableRoot->things->type->name, SymtableRoot->things->name, SymtableRoot->things->category); */
	/* printf("\nType of the second variable: %s\nName of it: %s\nCategory of it: %d\n", SymtableRoot->things->next->type->name, SymtableRoot->things->next->name, SymtableRoot->things->next->category); */
	/* printf("\nType of function: %s\nName of it: %s\n", FunctionRoot->next->type->name, FunctionRoot->next->name); */
	/* printf("\nType of the second function: %s\nName of it: %s\n", FunctionRoot->next->next->type->name, FunctionRoot->next->next->name); */
	/* printf("\nType of the var inside main func: %s\nName of it: %s\nCategory of it: %d\n", SymtableRoot->children[0]->things->type->name, SymtableRoot->children[0]->things->name, SymtableRoot->children[0]->things->category); */
	/* printf("\nType of the second var inside the main func: %s\nName of it: %s\nCategory of it: %d\n", SymtableRoot->children[0]->things->next->type->name, SymtableRoot->children[0]->things->next->name, SymtableRoot->children[0]->things->next->category); */
	/* printf("\nType of the var inside the second func: %s\nName of it: %s\nCategory of it: %d\n", SymtableRoot->children[1]->things->type->name, SymtableRoot->children[1]->things->name, SymtableRoot->children[1]->things->category); */
	
	printf("\n-----------------\n");
	printf("    parsing      ");
	printf("\n-----------------\n");
	printf("Root node\n\n");
	ReadParseTree(ParseRoot);

	if (AsmMake(argv[1]) == ERROR) {
		return ERROR;
	}
       
	ReadAsm();
	
	printf("%c", '\n');
	fclose(file);
	fclose(out);
	return SUCCESS;
}
