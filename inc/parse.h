#ifndef PARSE
#define PARSE

#include <main.h>

typedef enum NodeType_e { nret, nadd, nsub, nmult, ndivide, nca, /* <= Without "args" in the struct */ ndf, nnum, nav, nrv, nga, naa, nase, nasea, nrse, nrsea, nroot } NodeType;

typedef enum SymTableEntryType_e { tVar, tParam, tStruct } SymTableEntryType;

typedef struct typelist_s {
	char* name;
	char size;
	char isASize;
	struct typelist_s *next;
} TypeList;

typedef struct parsetable_s {
	struct parsetable_s *children;
	NodeType type;
	void* args;
	struct parsetable_s *next;
} ParseTable;

typedef struct symtableEntry_s {
	char* name;
	void *arg;
	int size;
	SymTableEntryType type;
	struct symtableEntry_s *next;
} SymTableEntry;

typedef struct symtable_s {
	SymTableEntry *things;
	struct symtable_s *children;
	struct symtable_s *next;
	int nPushes;
} SymTable;

typedef struct structTableEntry_s {
	TypeList *type;
	char *name;
	struct structTableEntry_s *next;
} StructTableEntry;

typedef struct structTable_s {
	char *name;
	StructTableEntry *elements;
	struct structTable_s *next;
} StructTable;

typedef struct functionTable_s {
	char *name;
	int nParams;
	struct functionTable_s *next;
} FunctionTable;

extern ParseTable *parseRoot;
extern SymTable *symTableRoot;
extern SymTable **symTableFamily;
extern SymTable *curSymNode;

Status Parse();
void AddNewSymFamilyMem(SymTable *toAdd);
void ShrinkSymFamily();

#endif
