#ifndef GEN_BIN
#define GEN_BIN

#define REG_AX 0
#define REG_BX 3
#define REG_CX 1
#define REG_DX 2
#define REG_SP 4
#define REG_BP 5
#define REG_SI 6
#define REG_DI 7

#define REG_AX_OFFSET REG_AX + 100
#define REG_DI_OFFSET REG_DI + 100

#define BIN_PUSH 0x50
#define BIN_POP 0x58
#define BIN_RET 0xC3

#define BIN_MUL_RAX_REG 0xF7
#define BIN_DIV_RAX_REG 0xF7
#define BIN_XCHG_RAX_REG 0x90

#define BIN_MOV_REG8_IMM8 0xB0
#define BIN_MOV_REG_IMM 0xB8
#define BIN_MOV_REG_RM 0x8B
#define BIN_MOV_RM_REG 0x89
#define BIN_MOV_REG8_RM8 0x8A
#define BIN_MOV_RM8_REG8 0x88

#define BIN_MOVZX_REG_RM16 0xB70F
#define BIN_MOVZX_REG_RM8 0xB60F

#define BIN_XOR_REG8_REG8 0x32
#define BIN_XOR_REG_REG 0x33

#define BIN_ADD_REG_REG 0x01
#define BIN_ADD_REG_IMM8 0x83
#define BIN_ADD_REG_IMM32 0x81

#define BIN_SUB_REG_REG 0x29
#define BIN_SUB_REG_IMM8 0x83
#define BIN_SUB_REG_IMM32 0x81

int GetIntSize(int64_t data);

void GenBinPush(char reg);
void GenBinPop(char reg);
void GenBinXchg(char reg, char reg_sec);
void GenBinMov(char reg_dest, char reg_src, int64_t data, char sData);
void GenBinRet();
void GenBinAdd(char reg_dest, char reg_sec, int64_t data, char sData);
void GenBinSub(char reg_dest, char reg_src, int64_t data, char sData);
void GenBinMul(char reg);
void GenBinDiv(char reg);

#endif
