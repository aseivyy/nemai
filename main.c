#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum tokens { error, eof, eoc, equal, isequal, isnequal, greater, greater_equal, lesser, lesser_equal, obracket_body, cbracket_body, obracket_param, cbracket_param, add, sub, mult, divide, number, word, kif, kfi };

typedef char STATUS;

typedef struct {
	enum tokens token;
	void *symtable;
} TOKEN;

typedef struct symtableEntry_s {
	char* name;
	char id;
	char type;
	char category;
	struct symtableEntry_s *next;
} symtableEntry;

typedef struct lextableEntry_s {
	char* name;
	int id;
	struct symtableEntry_s *next;
} lextableEntry;

int c_id = 0;

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
	return word;
}

TOKEN Lex() {
	char c;
	do {
		c = fgetc(file);
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
		token.token = obracket_param;
		token.symtable = (void*) 0;
		return token;
	}

	if (c == ')') {
		TOKEN token;
		token.token = cbracket_param;
		token.symtable = (void*) 0;
		return token;
	}

	if (c == '[') {
		TOKEN token;
		token.token = obracket_body;
		token.symtable = (void*) 0;
		return token;
	}

	if (c == ']') {
		TOKEN token;
		token.token = cbracket_body;
		token.symtable = (void*) 0;
		return token;
	}

	if (c == ';') {
		TOKEN token;
		token.token = eoc;
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

STATUS LexSetup(char *filename) {
	file = fopen(filename, "r");

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
