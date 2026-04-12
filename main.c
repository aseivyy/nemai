#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum tokens { error, eof, equal, isequal };

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

#define ERROR 1
#define SUCCESS 0

FILE *file;

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
	
	printf("%c", c);

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
	printf("%d ", test.token);
	test = Lex();
	printf("%d ", test.token);
	test = Lex();
	printf("%d ", test.token);

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
