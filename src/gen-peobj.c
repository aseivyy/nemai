#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <gen-peobj.h>
#include <main.h>
#include <gen-bin.h>
#include <parse.h>

FILE *out;

CoffHeader coffHeader;

CoffSectionHeader coffTextHeader;
uint32_t sText = 0;

CoffSymbolEntry *coffSymbolEntries;
int nSymbols = 0;

char *coffString;
int sString = 4;

int16_t *dataSection = (void*) 0;
int nCharsDataSection = 0;

void NextSymNode() {
	int indexAfterLast = 0;
	while (symTableFamily[indexAfterLast] != (void*) 0) indexAfterLast++;

	if (symTableFamily[indexAfterLast - 1]->next == (void*) 0) {
		ShrinkSymFamily();
	} else {
		symTableFamily[indexAfterLast - 1] = symTableFamily[indexAfterLast - 1]->next;
		curSymNode = symTableFamily[indexAfterLast - 1];
	}
}

int GetVarsSize() {
	int totalSize = 0;
	SymTableEntry *sizeChecker = curSymNode->things;

	while (sizeChecker != (void*) 0) {
		if (sizeChecker->type == tParam) {
			sizeChecker = sizeChecker->next;
			continue;
		}
		
		if (sizeChecker->size % 8 == 0) {
			totalSize += sizeChecker->size;
		} else {
			totalSize += sizeChecker->size + (8 - (sizeChecker->size % 8));
		}
		
		sizeChecker = sizeChecker->next;
	}

	return totalSize;
}

int GetVarOffset(char *name) {
	int64_t varOffset = 0;
	SymTableEntry *sizeChecker;

	for (int i = 0; symTableFamily[i] != curSymNode; i++) {
		varOffset += symTableFamily[i]->nPushes * 8;

		for (sizeChecker = symTableFamily[i]->things; sizeChecker != (void*) 0; sizeChecker = sizeChecker->next) {
			if (sizeChecker->type == tParam) continue;
			
			if (sizeChecker->size % 8 == 0) {
				varOffset += sizeChecker->size;
			} else {
				varOffset += sizeChecker->size + (8 - (sizeChecker->size % 8));
			}
		}
	}
	
	varOffset += curSymNode->nPushes * 8;
	sizeChecker = curSymNode->things;

	while (sizeChecker != (void*) 0 && strcmp(name, sizeChecker->name) != 0) {
		if (sizeChecker->type == tParam) {
			sizeChecker = sizeChecker->next;
			continue;
		}

		if (sizeChecker->size % 8 == 0) {
			varOffset += sizeChecker->size;
		} else {
			varOffset += sizeChecker->size + (8 - (sizeChecker->size % 8));
		}

		sizeChecker = sizeChecker->next;
	}

	if (sizeChecker->type == tParam) {
		int64_t paramOffset = GetVarsSize();
		paramOffset += curSymNode->nPushes * 8;
		for (SymTableEntry *paramSizeChecker = curSymNode->things; paramSizeChecker != sizeChecker; paramSizeChecker = paramSizeChecker->next) paramOffset += 8;

		return paramOffset + 8;
	} else {
		return varOffset;
	}
}

int64_t GetNParseChildren(ParseTable *parent) {
	int64_t nChildren = 0;
	for (ParseTable *i = parent->children; i != (void*) 0; i = i->next) nChildren++;
	return nChildren;
}

int64_t GetStackSize(SymTable *endTable) {
	int64_t size = 0;
	
	for (int i = 0; i == 0 || symTableFamily[i - 1] != endTable; i++) {
		size += symTableFamily[i]->nPushes * 8;

		for (SymTableEntry *sizeChecker = symTableFamily[i]->things; sizeChecker != (void*) 0; sizeChecker = sizeChecker->next) {
			if (sizeChecker->type == tParam) continue;
			
			if (sizeChecker->size % 8 == 0) {
				size += sizeChecker->size;
			} else {
				size += sizeChecker->size + (8 - (sizeChecker->size % 8));
			}
		}
	}

	return size;
}

void GenPeObjProcess(ParseTable *curProcessed) {
	int nChildren = 0;
	int64_t caStackChange = 0;
	
	switch (curProcessed->type) {
	case ndf: {
		nSymbols++;

		coffSymbolEntries = (CoffSymbolEntry*) realloc(coffSymbolEntries, sizeof(CoffSymbolEntry) * nSymbols);
		coffSymbolEntries[nSymbols - 1].nameAll = 0;

		int varNameLength = strlen(((FunctionTable**) (curProcessed->args))[0]->name);
		if (varNameLength <= 8) {
			memcpy((char*) (coffSymbolEntries[nSymbols - 1].name), ((FunctionTable**) (curProcessed->args))[0]->name, 8);
		} else {
			coffString = (char*) realloc(coffString, sizeof(char) * (sString + varNameLength + 1));
			strcpy(coffString + (sString * sizeof(char)), ((FunctionTable**) (curProcessed->args))[0]->name);
			coffSymbolEntries[nSymbols - 1].oName = sString;
			sString += varNameLength + 1;
		}

		coffSymbolEntries[nSymbols - 1].osValue = sText;
		coffSymbolEntries[nSymbols - 1].xSection = 1;
		coffSymbolEntries[nSymbols - 1].type = 0x20;
		coffSymbolEntries[nSymbols - 1].class = 2;
		coffSymbolEntries[nSymbols - 1].nAux = 0;

		if (curSymNode == symTableRoot) {
			AddNewSymFamilyMem(symTableRoot->children);
		}

		for (int i = 1; i <= ((FunctionTable**) curProcessed->args)[0]->nParams && i <= 4; i++) {
			char curArgReg;
			switch (i) {
			case 1:
				curArgReg = REG_CX;
				break;
			case 2:
				curArgReg = REG_DX;
				break;
			case 3:
				curArgReg = REG_R8;
				break;
			default:
				curArgReg = REG_R9;
			}
				
			GenBinMov(REG_SP, curArgReg, i * 8, 8);
		}
		
		int scopeVarSize = GetVarsSize();
		if (scopeVarSize != 0) GenBinSub(REG_SP, 0, scopeVarSize, 8);
		
		break;
	}
	case nnum: {
		int sNum = (GetIntSize(*((int64_t*) (curProcessed->args))) == 8) ? 8 : 4;  
		GenBinMov(REG_AX, 0, *((int64_t*) (curProcessed->args)), sNum);
		break;
	}
	case nca: {
		caStackChange += GetNParseChildren(curProcessed) * 8;
		if ((GetStackSize(curSymNode) + caStackChange) % 16 == 0) caStackChange += 8;
		
		curSymNode->nPushes += caStackChange / 8;
		
		GenBinSub(REG_SP, 0, caStackChange, 8);
		break;
	}
	default: break;
	}

	int64_t caCurArg = 0;

	ParseTable *curProcessedChild = curProcessed->children;
	while (curProcessedChild != (void*) 0) {
		GenPeObjProcess(curProcessedChild);

		switch (curProcessed->type) {
		case nadd:
			GenBinPush(REG_AX);
			break;
		case nmult:
			GenBinPush(REG_AX);
			break;
		case nsub:
			GenBinPush(REG_AX);
			break;
		case ndivide:
			GenBinPush(REG_AX);
			break;
		case nasea:
			GenBinPush(REG_AX);
			break;
		case nca: {
			switch (caCurArg) {
			case 0 :
				GenBinMov(REG_DI, REG_AX, 0, 0);
				break;
			case 1:
				GenBinMov(REG_CX, REG_AX, 0, 0);
				break;
			case 2:
				GenBinMov(REG_DX, REG_AX, 0, 0);
				break;
			case 3:
				GenBinMov(REG_R8, REG_AX, 0, 0);
				break;
			case 4:
				GenBinMov(REG_R9, REG_AX, 0, 0);
				break;
			default:
				GenBinMov(REG_SP, REG_AX, (caCurArg - 1) * 8, 8);
			}
			
			caCurArg++;
			break;
		}
		default: break;
		}

		nChildren++;
		
		curProcessedChild = curProcessedChild->next;
	}

	switch (curProcessed->type) {
	case nadd:
		sText--;
		curSymNode->nPushes--;
		fseek(out, -1, SEEK_CUR);
		for (int i = 0; i < nChildren - 1; i++) {
			GenBinPop(REG_CX);
			GenBinAdd(REG_AX, REG_CX, 0, 0);
		}
		break;
	case nmult:
		sText--;
		curSymNode->nPushes--;
		fseek(out, -1, SEEK_CUR);
		for (int i = 0; i < nChildren - 1; i++) {
			GenBinPop(REG_CX);
			GenBinMul(REG_CX);
		}
		break;
	case nsub:
		sText--;
		curSymNode->nPushes--;
		fseek(out, -1, SEEK_CUR);
		for (int i = 0; i < nChildren - 2; i++) {
			GenBinPop(REG_CX);
			GenBinAdd(REG_AX, REG_CX, 0, 0);
		}
		GenBinPop(REG_CX);
		GenBinXchg(REG_AX, REG_CX);
		GenBinSub(REG_AX, REG_CX, 0, 0);
		break;
	case ndivide:
		sText--;
		curSymNode->nPushes--;
		fseek(out, -1, SEEK_CUR);
		for (int i = 0; i < nChildren - 2; i++) {
			GenBinPop(REG_CX);
			GenBinMul(REG_CX);
		}
		GenBinPop(REG_CX);
		GenBinXchg(REG_AX, REG_CX);
		GenBinDiv(REG_CX);
		break;
	case ndf:
		NextSymNode();
		break;
	case nav:
		GenBinMov(REG_SP, REG_AX, GetVarOffset(((SymTableEntry**) curProcessed->args)[0]->name), 8);
		break;
	case nrv:
		GenBinMov(REG_AX, REG_SP, GetVarOffset(((SymTableEntry**) curProcessed->args)[0]->name), 8);
		break;
	case nase:
		GenBinMov(REG_SP, REG_AX, GetVarOffset(((SymTableEntry**) curProcessed->args)[0]->name) + ((int64_t*) curProcessed->args)[1], ((int64_t*) curProcessed->args)[2]);
		break;
	case nrse:
		GenBinMov(REG_AX, REG_SP, GetVarOffset(((SymTableEntry**) curProcessed->args)[0]->name) + ((int64_t*) curProcessed->args)[1], ((int64_t*) curProcessed->args)[2]);
		break;
	case nasea:
		/* Remove last push as value is already in rax */
		sText--;
		curSymNode->nPushes--;
		fseek(out, -1, SEEK_CUR);

		GenBinPop(REG_DI);
		GenBinMov(REG_DI_OFFSET, REG_AX, ((int64_t*) curProcessed->args)[0], ((int64_t*) curProcessed->args)[1]);
		break;
	case nrsea:		
		GenBinMov(REG_AX, REG_AX_OFFSET, ((int64_t*) curProcessed->args)[0], ((int64_t*) curProcessed->args)[1]);
		break;
	case nca:
		GenBinCall(REG_DI);
		GenBinAdd(REG_SP, 0, caStackChange, 8);
		curSymNode->nPushes -= caStackChange / 8;
		break;
	case nret: {
		int scopeVarSize = GetVarsSize();
		if (scopeVarSize != 0) GenBinAdd(REG_SP, 0, scopeVarSize, 8);

		GenBinRet();
		break;
	}
	default: break;
	}
}

Status GenPeObj() {
	char *outName;
	outName = (char*) malloc(strlen(curFileName) + strlen(".obj") + 1);
	strcpy(outName, curFileName);
	strcat(outName, ".obj");

	out = fopen(outName, "wb");

	if (out == (void*) 0) {
		printf("nemai: \tCouldn't write output to a file\n");
		return ERROR;
	}

	int nSections = (dataSection == (void*) 0) ? 1 : 2;

	coffString = (char*) malloc(sizeof(char) * 5);

	coffHeader.machine = 0x8664;
	coffHeader.nSections = nSections;
	coffHeader.time = 0;
	coffHeader.oSymTable = 0;
	coffHeader.nSymbols = 0;
	coffHeader.sOptHeader = 0;
	coffHeader.flags = 0;

	strncpy((char*) &(coffTextHeader.name[0]), ".text\0\0\0", 8);
	coffTextHeader.sVirt = 0;
	coffTextHeader.oVirt = 0;
	coffTextHeader.sRaw = 0;
	coffTextHeader.oRaw = 0;
	coffTextHeader.oRelocations = 0;
	coffTextHeader.oLineNumbers = 0;
	coffTextHeader.nRelocations = 0;
	coffTextHeader.nLineNumbers = 0;
	coffTextHeader.flags = 0x00000020 | 0x20000000;

	fwrite(&coffHeader, sizeof(CoffHeader), 1, out);
	fwrite(&coffTextHeader, sizeof(CoffSectionHeader), 1, out);

	if (nSections >= 2) {
		CoffSectionHeader coffDataHeader;
		
		strncpy((char*) &(coffDataHeader.name[0]), ".data\0\0\0", 8);
		coffDataHeader.sVirt = 0;
		coffDataHeader.oVirt = 0;
		coffDataHeader.sRaw = nCharsDataSection * sizeof(int16_t);
		coffDataHeader.oRaw = 0;
		coffDataHeader.oRelocations = 0;
		coffDataHeader.oLineNumbers = 0;
		coffDataHeader.nRelocations = 0;
		coffDataHeader.nLineNumbers = 0;
		coffDataHeader.flags = 0x40;
					     
		fwrite(&coffDataHeader, sizeof(CoffSectionHeader), 1, out);
	}

	symTableFamily = (SymTable**) realloc(symTableFamily, sizeof(SymTable*));
	symTableFamily[0] = (void*) 0;
	AddNewSymFamilyMem(symTableRoot);
	GenPeObjProcess(parseRoot);

	fseek(out, sizeof(CoffHeader) + 16, SEEK_SET);
	fwrite(&sText, 4, 1, out);

	int32_t oText = sizeof(CoffHeader) + (nSections * sizeof(CoffSectionHeader));
	fwrite(&oText, 4, 1, out);

	if (nSections >= 2) {
		int32_t toWrite = sizeof(CoffHeader) + (nSections * sizeof(CoffSectionHeader)) + sText;
		
		fseek(out, sizeof(CoffHeader) + sizeof(CoffSectionHeader) + 20, SEEK_SET);
		fwrite(&toWrite, 4, 1, out);
		fseek(out, 0, SEEK_END);
		fwrite(dataSection, nCharsDataSection * sizeof(int16_t), 1, out);
	}

	if (nSymbols != 0) {
		int32_t toWrite = sizeof(CoffHeader) + (sizeof(CoffSectionHeader) * nSections) + sText + (nCharsDataSection * sizeof(int16_t));
			
		fseek(out, 8, SEEK_SET);
		fwrite(&toWrite, 4, 1, out);
		fwrite(&nSymbols, 4, 1, out);
		fseek(out, 0, SEEK_END);

		for (int i = 0; i < nSymbols; i++) {
			fwrite(&(coffSymbolEntries[i]), sizeof(CoffSymbolEntry), 1, out);
		}

		memcpy(coffString, &sString, 4);
		fwrite(coffString, sString, 1, out);
	}
	
	fclose(out);
	return SUCCESS;
}
