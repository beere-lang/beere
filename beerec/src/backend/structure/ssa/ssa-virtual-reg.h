#ifndef IR_VIRTUAL_REG_H
#define IR_VIRTUAL_REG_H

typedef struct VirtualReg VirtualReg;

typedef struct IRNode IRNode;

typedef enum
{
	VIRTUAL_REG_GENERALS,
	VIRTUAL_REG_FLOATS
}
VirtualRegType;

typedef enum
{
	VIRTUAL_REG_8_BITS,
	VIRTUAL_REG_16_BITS,
	VIRTUAL_REG_32_BITS,
	VIRTUAL_REG_64_BITS
}
VirtualRegSize;

typedef struct
{
	IRNode* block;
	IRNode* use;
}
IRInstructionUse;

typedef struct
{
	IRInstructionUse** elements;

	int length;
	int capacity;
}
IRInstructionUseList;

struct VirtualReg
{
	VirtualRegType type;
	VirtualRegSize size;

	int id;

	IRInstructionUse* def;
	IRInstructionUseList* uses;
};

#endif