#include <stdio.h>
#include <stdint.h>

#include <gen-bin.h>
#include <gen-peobj.h>
#include <parse.h>

const char SRC_16B = 0x66;

int GetIntSize(int64_t data) {
	if (data >> 8 == 0) return 1;
	if (data >> 16 == 0) return 2;
	if (data >> 32 == 0) return 4;
	return 8;
}

int GetIntBitSize(int64_t data) {
	for (int i = 1; i < 64; i++) {
		if (data >> i == 0) return i;
	}

	return 64;
}

void ProcessRex(char* rexVar, char size, char *reg, char *reg_sec) {
	if (size == 8) {
		*rexVar |= REX_W;
	}
	if ((*reg >= 8 && *reg <= 15) || (*reg_sec >= 8 && *reg_sec <= 15)) {
		*rexVar |= REX_REG;
		if (*reg >= 8) reg -= 8; else reg_sec -= 8;
	}
}

void GenBinXchg(char reg, char reg_sec) {
	if (reg == REG_AX || reg_sec == REG_AX) {
		char opcode = BIN_XCHG_RAX_REG + reg + reg_sec;
		char rex = REX_W;
		fwrite(&rex, 1, 1, out);
		fwrite(&opcode, 1, 1, out);
		sText += 2;
	}
}

void GenBinRet() {
	char opcode = BIN_RET;
	fwrite(&opcode, 1, 1, out);
	sText++;
}

void GenBinPush(char reg) {
	char opcode = BIN_PUSH + reg;
	fwrite(&opcode, 1, 1, out);
	sText++;
	curSymNode->nPushes++;
}

void GenBinPop(char reg) {
	char opcode = BIN_POP + reg;
	fwrite(&opcode, 1, 1, out);
	sText++;
	curSymNode->nPushes--;
}

void GenBinXor(char reg_dest, char reg_sec, char sReg) {
	char opcode = (sReg == 1) ? BIN_XOR_REG8_REG8 : BIN_XOR_REG_REG;
	char modrm = 0b11000000 | (reg_dest << 3) | reg_sec;
	fwrite(&opcode, 1, 1, out);
	fwrite(&modrm, 1, 1, out);
	sText += 2;
}

void GenBinMov(char reg_dest, char reg_src, int64_t data, char sReg) {
	char rex = 0;
	char opcode;

	ProcessRex(&rex, sReg, &reg_dest, &reg_src);

	/* offset based scenarios, like mov Xx, [Yx + Zx] */
	if (reg_dest == REG_SP || reg_src == REG_SP || reg_dest == REG_DI_OFFSET || reg_src == REG_AX_OFFSET) {
		if (reg_dest == REG_DI_OFFSET) reg_dest -= 100;
		else if (reg_src == REG_AX_OFFSET) reg_src -= 100;
		
		/* yes i finally found out how to align in emacs so I can finally comment without it being ugly */
		char baseReg;
		if (reg_dest == REG_SP || reg_src == REG_SP)		baseReg = REG_SP;
		else if (reg_dest == REG_DI)				baseReg = REG_DI;
		else if (reg_dest != REG_SP && reg_dest != REG_DI)	baseReg = reg_dest;
		else							baseReg = reg_src;

		char sDisp = GetIntBitSize(data);
		char modrm = 0;
		if (data == 0) {
			modrm = 0;
		} else if (sDisp <= 7) {
			modrm = 0b01000000;
		} else if (sDisp <= 31) {
			modrm = 0b10000000;
		}

		char sib;
		if (sDisp == 32) {
			sib = 0b00001000 | baseReg;
			sText += 2;
			GenBinMov(REG_CX, 0, data, 4);
		} else if (sDisp > 32) {
			sib = 0b00001000 | baseReg;
			sText += 2;
			GenBinMov(REG_CX, 0, data, 8);		
		} else if (reg_dest == REG_SP || reg_src == REG_SP) {
			sib = 0b00100000 | baseReg;
		} else {
			sib = 0;
			sText--;
		}

		if (reg_dest == REG_SP || reg_dest == REG_DI) {
			opcode = (sReg != 1) ? BIN_MOV_RM_REG : BIN_MOV_RM8_REG8;

			switch (sReg) { 
			case 8:
				fwrite(&rex, 1, 1, out);
				fwrite(&opcode, 1, 1, out);
				sText += 2;
				break;
			case 4:
				fwrite(&opcode, 1, 1, out);
				sText++;
				break;
			case 2:
				fwrite(&SRC_16B, 1, 1, out);
				fwrite(&opcode, 1, 1, out);
				sText += 2;
				break;
			case 1:
				fwrite(&opcode, 1, 1, out);
				sText++;
				break;
			default: break;
			}
		} else {
			int16_t bigOpcode = (sReg >= 4) ? BIN_MOV_REG_RM : ((sReg == 2) ? BIN_MOVZX_REG_RM16 : BIN_MOVZX_REG_RM8);
			switch (sReg) {
			case 8:
				fwrite(&rex, 1, 1, out);
				fwrite(&bigOpcode, 1, 1, out);
				sText += 2;
				break;
			case 4:
				fwrite(&bigOpcode, 1, 1, out);
				sText++;
				break;
			case 2:
				fwrite(&bigOpcode, 2, 1, out);
				sText += 2;
				break;
			case 1:				
				fwrite(&bigOpcode, 2, 1, out);
				sText += 2;
				break;
			default: break;
			}
		}

		modrm |= (reg_dest == REG_SP || reg_dest == REG_DI) ? ((reg_src << 3) | ((sib != 0) ? REG_SP : baseReg)) : ((reg_dest << 3) | ((sib != 0) ? REG_SP : baseReg));
		
		fwrite(&modrm, 1, 1, out);
		if (sib != 0) fwrite(&sib, 1, 1, out);

		if (data == 0) {
			sText += 2;
		} else if (sDisp <= 7) {
			fwrite(&data, 1, 1, out);
			sText += 3;
		} else if (sDisp <= 31) {
			fwrite(&data, 4, 1, out);
			sText += 6;
		}
		return;
	}
	
	if (sReg != 0 && data == 0) {
		GenBinXor(reg_dest, reg_dest, sReg);
		return;
	} 
	
	switch (sReg) {
	case 1:
		opcode = BIN_MOV_REG8_IMM8 + reg_dest;
		fwrite(&opcode, 1, 1, out);
		fwrite(&data, 1, 1, out);
		sText += 2;

		return;
	case 2:
		opcode = BIN_MOV_REG_IMM + reg_dest;
		fwrite(&SRC_16B, 1, 1, out);
		fwrite(&opcode, 1, 1, out);
		fwrite(&data, 2, 1, out);
		sText += 4;
		
		return;
	case 4:
		opcode = BIN_MOV_REG_IMM + reg_dest;
		fwrite(&opcode, 1, 1, out);
		fwrite(&data, 4, 1, out);
		sText += 5;
		
		return;
	case 8:
		opcode = BIN_MOV_REG_IMM + reg_dest;
		fwrite(&rex, 1, 1, out);
		fwrite(&opcode, 1, 1, out);
		fwrite(&data, 8, 1, out);
		sText += 10;

		return;
	}

	opcode = BIN_MOV_REG_RM;
	char modrm = 0b11000000 | (reg_dest << 3) | reg_src;
	rex |= REX_W;
	
	fwrite(&rex, 1, 1, out);
	fwrite(&opcode, 1, 1, out);
	fwrite(&modrm, 1, 1, out);
	sText += 3;
}

void GenBinAdd(char reg_dest, char reg_sec, int64_t data, char sData) {
	char modrm;
	char opcode;
	char rex = 0;

	ProcessRex(&rex, 8, &reg_dest, &reg_sec);
	
	if (sData == 0) {
		opcode = BIN_ADD_REG_REG;
		modrm = 0b11000000 | (reg_sec << 3) | reg_dest;
		fwrite(&rex, 1, 1, out);
		fwrite(&opcode, 1, 1, out);
		fwrite(&modrm, 1, 1, out);
		sText += 3;
	} else {
		int bitSizeData = GetIntBitSize(data);

		if (bitSizeData <= 7) {
			opcode = BIN_ADD_REG_IMM8;
			modrm = 0b11000000 | reg_dest;
			fwrite(&rex, 1, 1, out);
			fwrite(&opcode, 1, 1, out);
			fwrite(&modrm, 1, 1, out);
			fwrite(&data, 1, 1, out);
			sText += 4;
		} else if (bitSizeData <= 31) {
			opcode = BIN_ADD_REG_IMM32;
			modrm = 0b11000000 | reg_dest;
			fwrite(&rex, 1, 1, out);
			fwrite(&opcode, 1, 1, out);
			fwrite(&modrm, 1, 1, out);
			fwrite(&data, 4, 1, out);
			sText += 7;
		} else if (bitSizeData == 32) {
			GenBinMov(REG_CX, 0, data, 4);
			GenBinAdd(reg_dest, REG_CX, 0, 0);
		} else {
			GenBinMov(REG_CX, 0, data, 8);
			GenBinAdd(reg_dest, REG_CX, 0, 0);
		}
	}
}

void GenBinMul(char reg) {
	char modrm;
	char opcode;
	char rex = REX_W;
	
	opcode = BIN_MUL_RAX_REG;
	modrm = 0b11100000 | reg;
	fwrite(&rex, 1, 1, out);
	fwrite(&opcode, 1, 1, out);
	fwrite(&modrm, 1, 1, out);
	sText += 3;
}

void GenBinSub(char reg_dest, char reg_sec, int64_t data, char sData) {
	char modrm;
	char opcode;
	char rex = 0;

	ProcessRex(&rex, 8, &reg_dest, &reg_sec);
	
	if (sData == 0) {
		opcode = BIN_SUB_REG_REG;
		modrm = 0b11000000 | (reg_sec << 3) | reg_dest;
		fwrite(&rex, 1, 1, out);
		fwrite(&opcode, 1, 1, out);
		fwrite(&modrm, 1, 1, out);
		sText += 3;
	} else {
		int bitSizeData = GetIntBitSize(data);

		if (bitSizeData <= 7) {
			opcode = BIN_SUB_REG_IMM8;
			modrm = 0b11101000 | reg_dest;
			fwrite(&rex, 1, 1, out);
			fwrite(&opcode, 1, 1, out);
			fwrite(&modrm, 1, 1, out);
			fwrite(&data, 1, 1, out);
			sText += 4;
		} else if (bitSizeData <= 31) {
			opcode = BIN_SUB_REG_IMM32;
			modrm = 0b11101000 | reg_dest;
			fwrite(&rex, 1, 1, out);
			fwrite(&opcode, 1, 1, out);
			fwrite(&modrm, 1, 1, out);
			fwrite(&data, 4, 1, out);
			sText += 7;
		} else if (bitSizeData == 32) {
			GenBinMov(REG_CX, 0, data, 4);
			GenBinSub(reg_dest, REG_CX, 0, 0);
		} else {
			GenBinMov(REG_CX, 0, data, 8);
			GenBinSub(reg_dest, REG_CX, 0, 0);
		}
	}
}

void GenBinDiv(char reg) {
	char modrm;
	char opcode;
	char rex = REX_W;

	GenBinMov(REG_DX, 0, 0, 4);
	
	opcode = BIN_DIV_RAX_REG;
	modrm = 0b11110000 | reg;
	fwrite(&rex, 1, 1, out);
	fwrite(&opcode, 1, 1, out);
	fwrite(&modrm, 1, 1, out);
	sText += 3;
}

void GenBinCall(char reg) {
	char opcode = BIN_CALL;
	char modrm = 0b11010000 | reg;

	fwrite(&opcode, 1, 1, out);
	fwrite(&modrm, 1, 1, out);

	sText += 2;
}
	
