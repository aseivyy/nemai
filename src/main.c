#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <lexSetup.h>
#include <lex.h>
#include <parse.h>
#include <gen-peobj.h>

char *curFileName;
Status isCreatingObject = IS;

int main(int argc, char **argv) {
	if (argc < 2) {
		printf("nemai: \tNot enough command parameters provided");
		printf("\n\tUse \"nemai [params] filename\"\n");
		return ERROR;
	} else if (argc > 3) {
		printf("nemai: \tToo many command arguments specified");
		printf("\n\tUse \"nemai [params] filename\"\n");
	} else if (argc == 3) {
		if (argv[1][0] == '-') {
			if (argv[1][1] == 'n') {
				isCreatingObject = NIS;
			} else {
				printf("nemai: \tInvalid command parameter \"%c\"", (argv[1])[1]);
				return ERROR;
			}
		} else {
			printf("nemai: \tInvalid command parameter syntax");
			printf("\n\tUse \"nemai [params] filename\"\n");
			return ERROR;
		}
	}

	curFileName = argv[argc - 1];
		
	if (LexSetup() == ERROR) return ERROR;
	if (Parse() == ERROR) return ERROR;

	if (isCreatingObject == IS) {
		if (GenPeObj() == ERROR) return ERROR;
	}
	
	printf("\n");
	
	fclose(in);
	return SUCCESS;
}

