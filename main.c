#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum tokens { error, eof, equal, isequal, isnequal, greater, greater_equal, lesser, lesser_equal, obracket_block, cbracket_block, obracket_sub, cbracket_sub, add, sub, mult, divide, number, word, kif, kfi, ret, df, dv };
enum nodetypes { nroot, ndf, ndv, nassign };
enum types { tvar };

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

typedef struct parse_root_s {
	void *statement;
	struct root_s *next;
} parse_root;

typedef struct symtableEntry_s {
	char* name;
	typelist *type;
	enum types category;
	struct symtableEntry_s *next;
} symtableEntry;

typedef struct symtable_s {
	symtableEntry *things;
	struct symtable_s *children;
	int nthings;
} symtable;

typedef struct functionTable_s {
	char *name;
	typelist *type;
	struct functionTable_s *next;
} functionTable;

typelist *Typelist;
int nTypes;

parse_root *ParseRoot;

symtable *SymtableRoot;
symtable* *SymtableFamily;

functionTable *FunctionRoot;

int lexLine = 1;

/* writing this since i am sure i am going to forget it */
void* *ParseFamily;			/* array of all parents of currently processed thing */
enum nodetypes *ParseFamilyType;	/* array of types of each parent in array above */
int ParseFamilyMembers = 0;		/* number of all things inside of the array above */
int *ParseFamilyFilled;			/* number of filled children for each of things in parse family */

#define ERROR 1
#define SUCCESS 0

FILE *file;

char isNum(char c) {
	if (c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7' || c == '8' || c == '9') return 1;
	return 0;
}

int charToNum(char c) {
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
	} while (c == ' ' || c == '\n' || c == '\t');

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
		int num = charToNum(c);
		char next = fgetc(file);
		
		while (isNum(next) == 1) {
			num *= 10;
			num += charToNum(next);
			next = fgetc(file);
		}
		fseek(file, -1, SEEK_CUR);

		TOKEN token;
		token.token = number;
		token.symtable = (int*) malloc(sizeof(int));
		*((int*) token.symtable) = num;
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
	

STATUS Parse() {
	BaseTypeSetup();

	printf("\n");
	typelist *curTypelist = Typelist;
	for (int i = 0; i < nTypes; i++) {
		printf("%s with size %d,  ", curTypelist->name, curTypelist->size);
		curTypelist = curTypelist->next;
	}
	printf("\n\n");

	ParseRoot = (parse_root*) malloc(sizeof(parse_root));
	ParseFamily = (void**) malloc(sizeof(void*));
	ParseFamily[0] = ParseRoot;
	ParseFamilyType = (enum nodetypes*) malloc(sizeof(enum nodetypes));
	ParseFamilyType[0] = nroot;
	ParseFamilyMembers++;
	ParseFamilyFilled = (int*) malloc(sizeof(int));
	ParseFamilyFilled[0] = 0;

	SymtableRoot = (symtable*) malloc(sizeof(symtable));
	SymtableRoot->nthings = 0;
	symtable *curNode = SymtableRoot;

	SymtableFamily = (symtable**) malloc(sizeof(symtable*) * 2);
	SymtableFamily[0] = SymtableRoot;
	SymtableFamily[1] = (void*) 0;

	FunctionRoot = (functionTable*) malloc(sizeof(functionTable));
	FunctionRoot->next = (void*) 0;

	// I know a waste of memory I couldnt think of any better way, so I guess first entry will be just always set to empty for now
	
	for (TOKEN i = Lex(); i.token != eof && i.token != error; i = Lex()) {
		if (i.token == obracket_block) {
			TOKEN node = Lex();
			if (node.token == dv) {
				symtableEntry **lastEntry = &(curNode->things);
				for (int i = 0; i < curNode->nthings; i++) {
					lastEntry = &((*lastEntry)->next);
				}
				*lastEntry = (symtableEntry*) malloc(sizeof(symtableEntry));
				/* typedef struct symtableEntry_s { */
				/* 	char* name; */
				/* 	typelist *type; */
				/* 	enum types category; */
				/* 	struct symtableEntry_s *next; */
				/* } symtableEntry; */
				TOKEN type = Lex();
				if (type.token != word) {
					printf("nemai:Parse \tVariable type isn't a type on line %d", lexLine);
					printf("%c", '\n');
					return ERROR;
				}
				
				curTypelist = Typelist;
				int isAType = 0;
				
				for (int i = 0; i < nTypes; i++) {
					if (strcmp(type.symtable, curTypelist->name) == 0) {
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

				TOKEN VarName = Lex();
				if (VarName.token != word) {
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
						if(strcmp(curListEntry->name, VarName.symtable) == 0) {
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

				(*lastEntry)->name[0] = ((char*) (VarName.symtable))[0];
				int nchars;
				for (int i = 1; ((char*) (VarName.symtable))[i] != '\0'; i++) {
					(*lastEntry)->name = realloc((*lastEntry)->name, sizeof(char) * (i + 1));
					(*lastEntry)->name[i] = ((char*) (VarName.symtable))[i];
					nchars = i;
				}
				
				(*lastEntry)->name = realloc((*lastEntry)->name, sizeof(char) * (nchars + 2));
				(*lastEntry)->name[nchars + 1] = '\0';

				(*lastEntry)->category = tvar;

				TOKEN ending = Lex();
				if (ending.token != cbracket_block) {
					printf("nemai:Parse \tFunction \"df\" can only take 2 arguments, but given more on line %d", lexLine);
					printf("%c", '\n');
					return ERROR;
				}
				
				(curNode->nthings)++;
			} else if (node.token == df) {

				functionTable **lastNode = &(FunctionRoot);
				while ((*lastNode)->next != (void*) 0) {
					lastNode = &((*lastNode)->next);
				}
				lastNode = &((*lastNode)->next);
				
				(*lastNode) = (functionTable*) malloc(sizeof(functionTable));
				(*lastNode)->next = (void*) 0;
				
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
				
				/* while((SymtableFamily[i] != (void*) 0) && nameCheckFail == 0) { */
				/* 	int nodenthings = SymtableFamily[i]->nthings; */
				/* 	symtableEntry *curListEntry = SymtableFamily[i]->things; */
				/* 	for (int i = 0; i < nodenthings && nameCheckFail == 0; i++) { */
				/* 		if(strcmp(curListEntry->name, VarName.symtable) == 0) { */
				/* 			nameCheckFail++; */
				/* 		} */
				/* 		curListEntry = curListEntry->next; */
				/* 	} */
				/* 	i++; */
				/* } */

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
			}
		}
	}
	return SUCCESS;
}

STATUS LexSetup(char *filename) {
	file = fopen(filename, "r");
	lexLine = 1;
	
	if (file == NULL) {
		printf("nemai:LexSetup \tFile doesn't exist or couldn't be opened.");
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

	TOKEN test = Lex();
	while (test.token != eof && test.token != error) {
		printf("%d ", test.token);
		test = Lex();
	}
	printf("%c", '\n');

	LexSetup(argv[1]);

	if (Parse() == ERROR) {
		return ERROR;
	}

	printf("\nType of variable: %s\nName of it: %s\nCategory of it: %d\n", SymtableRoot->things->type->name, SymtableRoot->things->name, SymtableRoot->things->category);
	printf("\nType of the second variable: %s\nName of it: %s\nCategory of it: %d\n", SymtableRoot->things->next->type->name, SymtableRoot->things->next->name, SymtableRoot->things->next->category);
	printf("\nType of function: %s\nName of it: %s\n", FunctionRoot->next->type->name, FunctionRoot->next->name);
	printf("\nType of the second function: %s\nName of it: %s\n", FunctionRoot->next->next->type->name, FunctionRoot->next->next->name);
	
	/* TOKEN test = Lex(); */
	/* printf("\n%d ", test.token); */
	/* //printf("\nThe number readen is %d\n", (int) *((int*) test.symtable)); */
	/* test = Lex(); */
	/* printf("%d ", test.token); */
	/* printf("\nThe readen string is %s\n", (char*) test.symtable); */
	/* test = Lex(); */
	/* printf("%d ", test.token); */

	// some linked list testing

	/* symtableEntry *test1 = (symtableEntry*) malloc(sizeof(symtableEntry)); */
	/* symtableEntry *test2 = (symtableEntry*) malloc(sizeof(symtableEntry)); */
	/* if (1 == 1) { */
	/* 	char mmname[10] = "abcdefghi\n"; */
	/* 	char *mname; */
	/* 	mname = (char*) malloc(sizeof(char) * 11); */
	/* 	mname = mmname; */
	/* 	test1->name = mname; */
	/* } */

	/* char mname = 'a'; */
	/* test1->next = test2; */

	/* char *sth = (char*) malloc(sizeof(char) * 5); */
	/* sth = "abc\n"; */
	/* test2->name = sth; */
	/* test2->next = (void*) 0; */

	/* symtableEntry *curr = test1; */
	/* while (1) { */
	/* 	printf("%s", curr->name); */
	/* 	//curr = curr->next; */
	/* 	if (curr->next == (void*) 0) break; */
	/* 	curr = curr->next; */
	/* } */

	/* printf("%s", test1->next->name); */
	
	printf("%c", '\n');
	fclose(file);
	return SUCCESS;
}
