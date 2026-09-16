#include <stdio.h>

#include <main.h>
#include <lexSetup.h>
#include <lex.h>

FILE *in;

Status LexSetup() {
	in = fopen(curFileName, "r");
	lexLine = 1;
	
	if (in == NULL) {
		printf("\nnemai: \tFile \"%s\" doesn't exist or couldn't be opened\n", curFileName);
		return ERROR;
	}

	char c = fgetc(in);
	if (c == EOF) {
		printf("\nnemai: \tFile \"%s\" is empty\n", curFileName);
		return ERROR;
	}

	fseek(in, -1, SEEK_CUR);
	
	return SUCCESS;
}
