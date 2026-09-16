#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <main.h>
#include <lex.h>
#include <lexSetup.h>
#include <gen-peobj.h>

int lexLine;
char *dataSectionToPlace;

char IsNum(char c) {
	if (c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7' || c == '8' || c == '9') return IS;
	return NIS;
}

char IsLetter(char c) {
	if ((c >= 'a' && c <= 'z') || (c >= 'A' &&  c <= 'Z') || c == '_' || c == '^' || c == '?') return IS;
	return NIS;
}

TokenType DetectKeyword(char *tester) {
	if (strcmp(tester, "ret") == 0) return ret;
	if (strcmp(tester, "df") == 0) return df;
	if (strcmp(tester, "dv") == 0) return dv;
	if (strcmp(tester, "av") == 0) return av;
	if (strcmp(tester, "ga") == 0) return ga;
	if (strcmp(tester, "aa") == 0) return aa;
	if (strcmp(tester, "sp") == 0) return sp;
	if (strcmp(tester, "ds") == 0) return ds;
	if (strcmp(tester, "rse") == 0) return rse;
	if (strcmp(tester, "ase") == 0) return ase;
	if (strcmp(tester, "rsea") == 0) return rsea;
	if (strcmp(tester, "asea") == 0) return asea;
	if (strcmp(tester, "ca") == 0) return ca;
	return word;
}

int64_t CharToNum(char c) {
	return c - '0';
}

Token Lex() {
	char c;
	do {
		c = fgetc(in);
		if (c == '\n') lexLine++;
	} while (c == ' ' || c == '\n' || c == '\t' || c == ';');

	if (c == '#') {
		c = fgetc(in);
		while (c != '\n' && c != EOF) c = fgetc(in);
		return Lex();
	}

	if (c == '"') {
		char isSpecialEncoded = fgetc(in);
		char specialEncoding = fgetc(in);
		if (isSpecialEncoded == '&' && specialEncoding == 'E') {
			Token token;
			token.token = string;
			*((int*) (token.data)) = nCharsDataSection;
			
			c = fgetc(in);
		
			while (c != '"' && c != EOF) {
				nCharsDataSection++;
				dataSection = (int16_t*) realloc(dataSection, nCharsDataSection * sizeof(int16_t));
				dataSection[nCharsDataSection - 1] = (int16_t) c;
				c = fgetc(in);
			}

			nCharsDataSection++;
			dataSection = (int16_t*) realloc(dataSection, nCharsDataSection * sizeof(int16_t));
			dataSection[nCharsDataSection - 1] = (int16_t) '\0';
			
			return token;
		} else {
			printf("nemai: \tYou must specify string encoding on line %d\n", lexLine);
			fseek(in, -2, SEEK_CUR);
		}

		c = fgetc(in);
	}
	
	if (c == EOF) {
		Token token;
		token.token = eof;
		token.data = (void*) 0;
		return token;
	}

	if (c == '(') {
		Token token;
		token.token = obracket_block;
		token.data = (void*) 0;
		return token;
	}

	if (c == ')') {
		Token token;
		token.token = cbracket_block;
		token.data = (void*) 0;
		return token;
	}

	if (c == '[') {
		Token token;
		token.token = obracket_sub;
		token.data = (void*) 0;
		return token;
	}

	if (c == ']') {
		Token token;
		token.token = cbracket_sub;
		token.data = (void*) 0;
		return token;
	}

	if (c == '+') {
		Token token;
		token.token = add;
		token.data = (void*) 0;
		return token;
	}

	if (c == '-') {
		Token token;
		token.token = sub;
		token.data = (void*) 0;
		return token;
	}

	if (c == '*') {
		Token token;
		token.token = mult;
		token.data = (void*) 0;
		return token;
	}

	if (c == '/') {
		Token token;
		token.token = divide;
		token.data = (void*) 0;
		return token;
	}

	if (IsNum(c) == IS) {
		int64_t num = CharToNum(c);
		char next = fgetc(in);
		
		while (IsNum(next) == IS) {
			num *= 10;
			num += CharToNum(next);
			next = fgetc(in);
		}
		fseek(in, -1, SEEK_CUR);

		Token token;
		token.token = number;
		token.data = (int64_t*) malloc(sizeof(int64_t));
		*((int64_t*) token.data) = num;
		return token;
	}

	if (IsLetter(c) == IS) {
		char *read = (char*) malloc(sizeof(char));
		*read = c;
		int letters = 1;
		
		c = fgetc(in);

		while (IsLetter(c) == IS || IsNum(c) == IS) {
			letters++;
			read = (char*) realloc(read, letters);
			read[letters - 1] = c;

			c = fgetc(in);
		}

		letters++;
		read = (char*) realloc(read, letters);
		read[letters - 1] = '\0';

		fseek(in, -1, SEEK_CUR);

		TokenType check = DetectKeyword(read);
		if (check == word) {
			Token token;
			token.token = word;
			token.data = read;
			return token;
		} else {
			free(read);
			Token token;
			token.token = check;
			token.data = (void*) 0;
			return token;
		}
	}
	
	printf("nemai: \tUnsupported character \"%c\" on line %d", c, lexLine);

	Token token;
	token.token = error;
	return token;
}
