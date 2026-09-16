#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include <parse.h>
#include <main.h>
#include <lex.h>

TypeList *typeListRoot;
ParseTable *parseRoot;
SymTable *symTableRoot;
FunctionTable *functionRoot;
StructTable *structRoot;

ParseTable **parseFamily;
SymTable **symTableFamily;

SymTable *curSymNode;
ParseTable *curParseNode;

void BaseTypeSetup() {
	typeListRoot = (TypeList*) malloc(sizeof(TypeList));
	typeListRoot->name = (char*) malloc(sizeof(char) * 3);
	strcpy(typeListRoot->name, "^u");
	typeListRoot->size = 8;
	typeListRoot->isASize = 0;

	TypeList *b8 = (TypeList*) malloc(sizeof(TypeList));
	b8->name = (char*) malloc(sizeof(char) * 3);
	strcpy(b8->name, "^8");
	b8->size = 1;
	b8->isASize = 1;
	typeListRoot->next = b8;
	
	TypeList *b16 = (TypeList*) malloc(sizeof(TypeList));
	b16->name = (char*) malloc(sizeof(char) * 4);
	strcpy(b16->name, "^16");
	b16->size = 2;
	b16->isASize = 1;
	typeListRoot->next->next = b16;

	TypeList *b32 = (TypeList*) malloc(sizeof(TypeList));
	b32->name = (char*) malloc(sizeof(char) * 4);
	strcpy(b32->name, "^32");
	b32->size = 4;
	b32->isASize = 1;
	typeListRoot->next->next->next = b32;

	TypeList *b64 = (TypeList*) malloc(sizeof(TypeList));
	b64->name = (char*) malloc(sizeof(char) * 4);
	strcpy(b64->name, "^64");
	b64->size = 8;
	b64->isASize = 1;
	typeListRoot->next->next->next->next = b64;
	typeListRoot->next->next->next->next->next = (void*) 0;
}

TypeList* FindType(char* typeName) {
	TypeList *curTypeList = typeListRoot;

	while (curTypeList != (void*) 0) {
		if (strcmp(typeName, curTypeList->name) == 0) return curTypeList;
		curTypeList = curTypeList->next;
	}

	return (void*) 0;
}

StructTable* FindStructPrototype(char *name) {
	if (structRoot->next == (void*) 0) {
		if (strcmp(structRoot->name, name) == 0) return structRoot;
		return (void*) 0;
	}

	StructTable *foundEntry = structRoot;
	for (; foundEntry != (void*) 0 && strcmp(name, foundEntry->name) != 0; foundEntry = foundEntry->next);

	return foundEntry;
}
			
void AddNewSymFamilyMem(SymTable *toAdd) {
	int indexToPlace = 0;
	while (symTableFamily[indexToPlace] != (void*) 0) indexToPlace++;

	symTableFamily = (SymTable**) realloc(symTableFamily, sizeof(SymTable*) * (indexToPlace + 2));
	symTableFamily[indexToPlace] = toAdd;
	symTableFamily[indexToPlace + 1] = (void*) 0;

	curSymNode = toAdd;
}

void ShrinkSymFamily() {
	int indexAfterLast = 0;
	while (symTableFamily[indexAfterLast] != (void*) 0) indexAfterLast++;

	symTableFamily = (SymTable**) realloc(symTableFamily, sizeof(SymTable*) * indexAfterLast);
	symTableFamily[indexAfterLast - 1] = (void*) 0;

	curSymNode = symTableFamily[indexAfterLast - 2];
}

void PlaceNewSymChild() {
	if (curSymNode->children == (void*) 0) {
		curSymNode->children = (SymTable*) malloc(sizeof(SymTable));
		curSymNode->children->next = (void*) 0;
		curSymNode->children->children = (void*) 0;
		curSymNode->children->things = (void*) 0;
		AddNewSymFamilyMem(curSymNode->children);
	} else {
		SymTable *lastSym = curSymNode->children;
		while (lastSym->next != (void*) 0) lastSym = lastSym->next;
		
		lastSym->next = (SymTable*) malloc(sizeof(SymTable));
		lastSym->next->next = (void*) 0;
		lastSym->next->children = (void*) 0;
		lastSym->next->things = (void*) 0;
		AddNewSymFamilyMem(lastSym->next);
	}
}

SymTableEntry* CheckSymNameInFamily(char *thingName) {
	for (int i = 0; symTableFamily[i + 1] != (void*) 0; i++) {
		SymTableEntry *nameChecker = symTableFamily[i]->things;
		while (nameChecker != (void*) 0) {
			if (strcmp(nameChecker->name, thingName) == 0) return nameChecker;
			nameChecker = nameChecker->next;
		}
	}

	return (void*) 0;
}

SymTableEntry* FindSymThing(char *thingName) {
	SymTableEntry *foundEntry = CheckSymNameInFamily(thingName);

	if (foundEntry == (void*) 0) {
		foundEntry = curSymNode->things;

		while (foundEntry != (void*) 0 && strcmp(foundEntry->name, thingName) != 0) {
			foundEntry = foundEntry->next;
		}
	}

	return foundEntry;
}	

SymTableEntry* AddNewSymThing(char *thingName) {
	if (CheckSymNameInFamily(thingName) != (void*) 0) return (void*) 0;
	if (curSymNode->things == (void*) 0) {
		curSymNode->things = (SymTableEntry*) malloc(sizeof(SymTableEntry));
		curSymNode->things->name = thingName;
		curSymNode->things->next = (void*) 0;
		return curSymNode->things;
	} else {
		SymTableEntry *lastThing = curSymNode->things;
		while (lastThing->next != (void*) 0) {
			if (strcmp(lastThing->name, thingName) == 0) return (void*) 0;
			lastThing = lastThing->next;
		}

		if (strcmp(lastThing->name, thingName) == 0) return (void*) 0;

		lastThing->next = (SymTableEntry*) malloc(sizeof(SymTableEntry));
		lastThing->next->name = thingName;
		lastThing->next->next = (void*) 0;

		return lastThing->next;
	}
}
			
void AddNewParseFamilyMem(ParseTable *toAdd) {
	int indexToPlace = 0;
	while (parseFamily[indexToPlace] != (void*) 0) indexToPlace++;

	parseFamily = (ParseTable**) realloc(parseFamily, sizeof(ParseTable*) * (indexToPlace + 2));
	parseFamily[indexToPlace] = toAdd;
	parseFamily[indexToPlace + 1] = (void*) 0;

	curParseNode = toAdd;
}

void ShrinkParseFamily() {
	int indexAfterLast = 0;
	while (parseFamily[indexAfterLast] != (void*) 0) indexAfterLast++;

	parseFamily = (ParseTable**) realloc(parseFamily, sizeof(ParseTable*) * indexAfterLast);
	parseFamily[indexAfterLast - 1] = (void*) 0;

	curParseNode = parseFamily[indexAfterLast - 2];
}

ParseTable* PlaceNewParseChild() {
	if (curParseNode->children == (void*) 0) {
		curParseNode->children = (ParseTable*) malloc(sizeof(ParseTable));
		curParseNode->children->next = (void*) 0;
		curParseNode->children->children = (void*) 0;
		return curParseNode->children;
	} else {
		ParseTable *lastParse = curParseNode->children;
		while (lastParse->next != (void*) 0) lastParse = lastParse->next;
		
		lastParse->next = (ParseTable*) malloc(sizeof(ParseTable));
		lastParse->next->next = (void*) 0;
		lastParse->next->children = (void*) 0;
		return lastParse->next;
	}
}

Status AddVar(Token *varType, Token *varName) {
	TypeList *foundTypeList = FindType((char*) varType->data);
	if (foundTypeList == (void*) 0) {
		printf("nemai: \tVariable type isn't a registered type on line %d\n", lexLine);
		return ERROR;
	}

	SymTableEntry *newThing = AddNewSymThing((char*) varName->data);
	if (newThing == (void*) 0) {
		printf("nemai: \tAttempt to redefine a variable on line %d\n", lexLine);
		return ERROR;
	}

	newThing->arg = foundTypeList;
	newThing->size = foundTypeList->size;
	newThing->type = tVar;

	return SUCCESS;
}

struct sGetStructTableElementData {
	int64_t offset;
	char elemSize;
};

struct sGetStructTableElementData* GetStructTableElementData(StructTable *structElemSrc, char *elemName) {
	struct sGetStructTableElementData *returned = malloc(sizeof(*returned));
	returned->offset = 0;
	
	StructTableEntry *nameTester = structElemSrc->elements;

	while (nameTester != (void*) 0) {
		if (strcmp(elemName, nameTester->name) == 0) {
			returned->elemSize = nameTester->type->size;
			return returned;
		}
		
		returned->offset += nameTester->type->size;
		nameTester = nameTester->next;
	}

	return (void*) 0;
}
	
				       
Status Parse() {
	BaseTypeSetup();

	parseRoot = (ParseTable*) malloc(sizeof(ParseTable));
	parseRoot->children = (void*) 0;
	parseRoot->next = (void*) 0;
	parseRoot->type = nroot;

	parseFamily = (ParseTable**) malloc(sizeof(ParseTable*) * 2);
	parseFamily[0] = parseRoot;
	parseFamily[1] = (void*) 0;

	curParseNode = parseRoot;

	symTableRoot = (SymTable*) malloc(sizeof(SymTable));
	symTableRoot->children = (void*) 0;
	symTableRoot->things = (void*) 0;

	symTableFamily = (SymTable**) malloc(sizeof(SymTable*) * 2);
	symTableFamily[0] = symTableRoot;
	symTableFamily[1] = (void*) 0;

	curSymNode = symTableRoot;

	functionRoot = (FunctionTable*) malloc(sizeof(FunctionTable));
	functionRoot->name = (char*) 0;
	functionRoot->next = (void*) 0;

	structRoot = (StructTable*) malloc(sizeof(StructTable));
	structRoot->next = (void*) 0;
	structRoot->name = (char*) 0;
	structRoot->elements = (void*) 0;

	for (Token token = Lex(); token.token != eof && token.token != error; token = Lex()) {
		switch (token.token) {
		case obracket_block: {
			Token func = Lex();			
			if (func.token == df) {
				if (curSymNode != symTableRoot) {
					printf("nemai: \tAttempt to define a function inside of a function on line %d\n", lexLine);
					return ERROR;
				}
				
				PlaceNewSymChild();
			}

			if (func.token != dv && func.token != sp && func.token != ds) {
				ParseTable *newParse = PlaceNewParseChild();
				AddNewParseFamilyMem(newParse);
			}

			if (func.token >= ret && func.token <= ca) {
				curParseNode->type = (int) func.token;
				break;
			}

			/* Used in rse rsea ase asea functions because they share code and error messages muct be different */
			char action[] = "assign to";

			switch (func.token) {
			case df: {
				curParseNode->type = ndf;
				curParseNode->args = (FunctionTable**) malloc(sizeof(FunctionTable*));

				Token funcName = Lex();

				if (funcName.token != word) {
					printf("nemai: \tFunction name is reserved or forbidden on line %d\n", lexLine);
					return ERROR;
				}

				Status nameExisting = NIS;
				FunctionTable *lastFunc = functionRoot;
				FunctionTable *placeLoc;
				
				if (functionRoot->name == (char*) 0) {
					placeLoc = functionRoot;
				} else {
					while (lastFunc->next != (void*) 0) {
						if (strcmp(funcName.data, lastFunc->name) == 0) {
							nameExisting = IS;
							lastFunc->next = (void*) 0;
						} else lastFunc = lastFunc->next;
					}

					if (nameExisting == IS || strcmp(funcName.data, lastFunc->name) == 0) {
						printf("nemai: \tAttempt to redefine a function on line %d\n", lexLine);
						return ERROR;
					}

					lastFunc->next = (FunctionTable*) malloc(sizeof(FunctionTable));
					placeLoc = lastFunc->next;
				}
				
				placeLoc->next = (void*) 0;
				placeLoc->name = funcName.data;
				placeLoc->nParams = 0;			       

				Token bracketCheck = Lex();
				if (bracketCheck.token != obracket_sub) {
					printf("nemai: \tMissing parameter list in function definition on line %d\n", lexLine);
					return ERROR;
				}

				Token paramType = Lex();

				if (paramType.token != cbracket_sub) {
					Token paramName = Lex();
					while (paramType.token == word && paramName.token == word) {
						if (AddVar(&paramType, &paramName) == ERROR) return ERROR;
						placeLoc->nParams++;

						paramType = Lex();
						if (paramType.token != cbracket_sub) paramName = Lex();
					}
				}

				((FunctionTable**) curParseNode->args)[0] = placeLoc;
				
				break;
			}
			case dv: {
				Token varType = Lex();
				Token varName = Lex();

				if (varType.token != word) {
					printf("nemai: \tVariable type isn't a type on line %d\n", lexLine);
					return ERROR;
				} else if (varName.token != word) {
					printf("nemai: \tVariable name is reserved or forbidden on line %d\n", lexLine);
					return ERROR;
				}

				if (AddVar(&varType, &varName) == ERROR) return ERROR;

				Lex();

				break;
			}
			case av: {
				curParseNode->type = nav;
				curParseNode->args = (SymTableEntry**) malloc(sizeof(SymTableEntry*));

				Token varName = Lex();
				if (varName.token != word) {
					printf("nemai: \tAttempt to assign to a variable with a reserved or forbidden name on line %d\n", lexLine);
					return ERROR;
				}

				SymTableEntry *foundThing = FindSymThing((char*) varName.data);

				if (foundThing == (void*) 0) {
					printf("nemai: \tAttempt to assign to an undefined variable on line %d\n", lexLine);
					return ERROR;
				}

				((SymTableEntry**) curParseNode->args)[0] = foundThing;

				break;
			}
			case sp: {
				Token protName = Lex();
				if (protName.token != word) {
					printf("nemai: \tStruct prototype name is reserved or forbidden on line %d\n", lexLine);
					return ERROR;
				}

				StructTable *placeLoc = structRoot;

				if (structRoot->name == (char*) 0) {
					placeLoc = structRoot;
				} else {					
					while (placeLoc->next != (void*) 0) {
						if (strcmp(protName.data, placeLoc->name) == 0) {
							placeLoc = (void*) 0;
							break;
						} else placeLoc = placeLoc->next;
					}

					if (placeLoc == (void*) 0 || strcmp(protName.data, placeLoc->name) == 0) {
						printf("nemai: \tAttempt to redefine a struct prototype on line %d\n", lexLine);
						return ERROR;
					}

					placeLoc->next = (StructTable*) malloc(sizeof(StructTable));
					placeLoc = placeLoc->next;
				}
				
				placeLoc->next = (void*) 0;
				placeLoc->name = protName.data;

				Token selemType = Lex();

				if (selemType.token != cbracket_block) {
					Token selemName = Lex();
					while (selemType.token == word && selemName.token == word) {
						TypeList *foundTypeList = FindType((char*) selemType.data);
						if (foundTypeList == (void*) 0) {
							printf("nemai: \tStruct prototype element type isn't a registered type on line %d\n", lexLine);
							return ERROR;
						}

						StructTableEntry *selemPlaceLoc = placeLoc->elements;
						if (selemPlaceLoc == (void*) 0) {
							placeLoc->elements = (StructTableEntry*) malloc(sizeof(StructTableEntry));
							selemPlaceLoc = placeLoc->elements;
						} else {
							Status isNameExisting = NIS;
							while (selemPlaceLoc->next != (void*) 0) {
								if (strcmp(selemName.data, selemPlaceLoc->name) == 0) {
									isNameExisting = IS;
									break;
								}
								
								selemPlaceLoc = selemPlaceLoc->next;
							}

							if (isNameExisting == IS || strcmp(selemName.data, selemPlaceLoc->name) == 0) {
								printf("nemai: \tAttempt to redefine a struct prototype element on line %d\n", lexLine);
								return ERROR;
							}

							selemPlaceLoc->next = (StructTableEntry*) malloc(sizeof(StructTableEntry));
							selemPlaceLoc = selemPlaceLoc->next;
						}

						selemPlaceLoc->next = (void*) 0;
						selemPlaceLoc->name = selemName.data;
						selemPlaceLoc->type = foundTypeList;

						selemType = Lex();
						if (selemType.token != cbracket_block) selemName = Lex();
					}
				}
				
				break;
			}
			case ds: {
				Token spName = Lex();
				if (spName.token != word) {
					printf("nemai: \tAttempt to define a struct from forbidden struct prototype name on line %d\n", lexLine);
					return ERROR;
				}

				StructTable *structProt = FindStructPrototype(spName.data);
				if (structProt == (void*) 0) {
					printf("nemai: \tAttempt to define a struct from undefined struct prototype on line %d\n", lexLine);
					return ERROR;
				}

				Token structName = Lex();
				if (spName.token != word) {
					printf("nemai: \tAttempt to define a struct with a forbidden name on line %d\n", lexLine);
					return ERROR;
				}
				
				SymTableEntry *newEntry = AddNewSymThing(structName.data);
				if (newEntry == (void*) 0) {
					printf("nemai: \tAttempt to redefine a struct on line %d\n", lexLine);
					return ERROR;
				}

				int selemsSize = 0;
				for (StructTableEntry *sizeAdder = structProt->elements; sizeAdder != (void*) 0; sizeAdder = sizeAdder->next) selemsSize += sizeAdder->type->size;

				newEntry->size = selemsSize;
				newEntry->arg = structProt;
				newEntry->type = tStruct;

				Token cBracket = Lex();
				if (cBracket.token != cbracket_block) {
					printf("nemai: \tToo many arguments specified when defining a struct on line %d\n", lexLine);
					return ERROR;
				}
				
				break;
			}
			case rse:
				strcpy(action, "read from");
			case ase: {
				curParseNode->type = (func.token == ase) ? nase : nrse;
				curParseNode->args = (void**) malloc(sizeof(void*) * 3);
				
				Token structName = Lex();
				if (structName.token != word) {
					printf("nemai: \tAttempt to %s a struct with forbidden name on line %d\n", action, lexLine);
					return ERROR;
				}
				
				SymTableEntry *foundThing = FindSymThing((char*) structName.data);
				if (foundThing == (void*) 0) {
					printf("nemai: \tAttempt to %s an undefined struct on line %d\n", action, lexLine);
					return ERROR;
				}

				if (foundThing->type != tStruct) {
					printf("nemai: \tAttempt to %s a struct which is not a struct on line %d\n", action, lexLine);
					return ERROR;
				}

				Token elemName = Lex();
				if (elemName.token != word) {
					printf("nemai: \tAttempt to %s a struct element with forbidden name on line %d\n", action, lexLine);
					return ERROR;
				}

				struct sGetStructTableElementData *elemData = GetStructTableElementData((StructTable*) (foundThing->arg), elemName.data);
				if (elemData == (void*) 0) {
					printf("nemai: \tAttempt to %s a struct element that doesn't exist in the struct prototype on line %d\n", action, lexLine);
					return ERROR;
				}

				((SymTableEntry**) curParseNode->args)[0] = foundThing;
				((int64_t*) curParseNode->args)[1] = elemData->offset;
				((int64_t*) curParseNode->args)[2] = elemData->elemSize;
				
				break;
			}
			case rsea:
				strcpy(action, "read from"); 
			case asea: {
				curParseNode->type = (func.token == asea) ? nasea : nrsea;
				curParseNode->args = (void**) malloc(sizeof(void*) * 2);
				
				Token spName = Lex();
				if (spName.token != word) {
					printf("nemai: \tAttempt to %s an address as struct with a forbidden struct prototype name on line %d\n", action, lexLine);
					return ERROR;
				}

				StructTable *foundSp = FindStructPrototype(spName.data);
				if (foundSp == (void*) 0) {
					printf("nemai: \tAttempt to %s an address as struct with an undefined struct prototype name on line %d\n", action, lexLine);
					return ERROR;
				}

				Token selemName = Lex();
				if (selemName.token != word) {
					printf("nemai: \tAttempt to %s an address as struct with a forbidden struct prototype element name on line %d\n", action, lexLine);
					return ERROR;
				}

				struct sGetStructTableElementData *selemData =  GetStructTableElementData(foundSp, selemName.data);
				if (selemData == (void*) 0) {
					printf("nemai: \tAttempt to %s an address as struct with an undefined struct prototype element name on line %d\n", action, lexLine);
					return ERROR;
				}

				((int64_t*) curParseNode->args)[0] = selemData->offset;
				((int64_t*) curParseNode->args)[1] = selemData->elemSize;
				
				break;
			}
			default:
				printf("nemai: \tAttempt to call an unknown function on line %d\n", lexLine);
				return ERROR;
			}
				
			break;
		}
		case cbracket_block: {
			if (curParseNode->type == ndf) {
				ShrinkSymFamily();
			}			
			
			ShrinkParseFamily();
							
			break;
		}
		default: {
			ParseTable *toPlace = PlaceNewParseChild();

			switch (token.token) {
			case number:
				toPlace->args = (int64_t*) malloc(sizeof(int64_t));
				toPlace->type = nnum;
				((int64_t*) (toPlace->args))[0] = *((int64_t*) token.data);
				break;
			case word:
				toPlace->type = nrv;
				toPlace->args = (SymTableEntry**) malloc(sizeof(SymTableEntry*));

				SymTableEntry *foundThing = FindSymThing((char*) token.data);

				if (foundThing == (void*) 0) {
					printf("nemai: \tAttempt to read from an undefined variable on line %d\n", lexLine);
					return ERROR;
				}

				((SymTableEntry**) toPlace->args)[0] = foundThing;

				break;
			default: break;
			}
		}
		}
	}

	return SUCCESS;
}
