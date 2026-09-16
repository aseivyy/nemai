#ifndef GEN_PEOBJ
#define GEN_PEOBJ

#include <stdint.h>
#include <stdio.h>

#include <main.h>

typedef struct {
	uint16_t machine;
	uint16_t nSections;
	uint32_t time;
	uint32_t oSymTable;
	uint32_t nSymbols;
	uint16_t sOptHeader;
	uint16_t flags;
} CoffHeader;

typedef struct {
	uint8_t name[8];
	uint32_t sVirt;
	uint32_t oVirt;
	uint32_t sRaw;
	uint32_t oRaw;
	uint32_t oRelocations;
	uint32_t oLineNumbers;
	uint16_t nRelocations;
	uint16_t nLineNumbers;
	uint32_t flags;
} __attribute__((packed)) CoffSectionHeader;

typedef struct {
	union {
		struct {
			uint32_t zero;
			uint32_t oName;
		};
		uint8_t name[8];
		uint64_t nameAll;
	};
	
	uint32_t osValue;
	uint16_t xSection;
	uint16_t type;
	uint8_t class;
	uint8_t nAux;
} __attribute__((packed)) CoffSymbolEntry;

extern FILE *out;
extern int16_t *dataSection;
extern int nCharsDataSection;
extern uint32_t sText;

Status GenPeObj();

#endif
