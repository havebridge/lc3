#include "include/vm.h"
#include "include/opcodes.h"


#define DestinationAddress(opcode) ((opcode >> 9) & 0x7)
#define SourceAddress1(opcode) ((opcode >> 6) & 0x7)
#define BaseRegAddress(opcode) ((opcode >> 6) & 0x7)

void ADD_op(vm* lc)
{
	if (((lc->opcode >> 5) & 0x1))
	{
		lc->generalReg[DestinationAddress(lc->opcode)] = lc->generalReg[SourceAddress1(lc->opcode)] + sign_extend(lc->opcode & 0x001F, 5);
	}
	else
	{
		lc->generalReg[DestinationAddress(lc->opcode)] = lc->generalReg[SourceAddress1(lc->opcode)] + lc->generalReg[lc->opcode & 0x7];
	}

	update_flags(lc, DestinationAddress(lc->opcode));
}

void AND_op(vm* lc)
{
	uint16_t dr = DestinationAddress(lc->opcode);
	uint16_t sr1 = SourceAddress1(lc->opcode);

	if ((lc->opcode >> 5) & 0x1)
	{
		uint16_t imm5 = sign_extend(lc->opcode & 0x001F, 5);
		lc->generalReg[dr] = lc->generalReg[sr1] & imm5;
	}
	else
	{
		uint16_t sr2 = lc->opcode & 0x7;
		lc->generalReg[dr] = lc->generalReg[sr1] & lc->generalReg[sr2];
	}

	update_flags(lc, dr);
}

void BR_op(vm* lc)
{
	uint16_t flags = (lc->opcode >> 9) & 0x7;
	if (flags & lc->condReg)
	{
		uint16_t pc_offset = sign_extend(lc->opcode & 0x01FF, 9);
		lc->pcReg += pc_offset;
	}
}

void JMP_op(vm* lc)
{
	lc->pcReg = lc->generalReg[BaseRegAddress(lc->opcode)];
}

void JSR_op(vm* lc)
{
	lc->generalReg[0x7] = lc->pcReg;
	if (((lc->opcode >> 11) & 0x1))
	{
		uint16_t pc_offset = sign_extend(lc->opcode & 0x07FF, 11);
		lc->pcReg += pc_offset;
	}
	else
	{
		lc->pcReg = lc->generalReg[BaseRegAddress(lc->opcode)];
	}
}

void LD_op(vm* lc)
{
	lc->generalReg[DestinationAddress(lc->opcode)] = memory_read(lc, lc->pcReg + sign_extend(lc->opcode & 0x01FF, 9));
	update_flags(lc, DestinationAddress(lc->opcode));
}

void LDI_op(vm* lc)
{
	lc->generalReg[DestinationAddress(lc->opcode)] = memory_read(lc, memory_read(lc, lc->pcReg + sign_extend(lc->opcode & 0x01FF, 9)));
	update_flags(lc, DestinationAddress(lc->opcode));
}

void LDR_op(vm* lc)
{
	lc->generalReg[DestinationAddress(lc->opcode)] = memory_read(lc, lc->generalReg[BaseRegAddress(lc->opcode)] + sign_extend(lc->opcode & 0x003F, 6));
	update_flags(lc, DestinationAddress(lc->opcode));
}

void LEA_op(vm* lc)
{
	lc->generalReg[DestinationAddress(lc->opcode)] = lc->pcReg + sign_extend(lc->opcode & 0x01FF, 9);
	update_flags(lc, DestinationAddress(lc->opcode));
}

void NOT_op(vm* lc)
{
	lc->generalReg[DestinationAddress(lc->opcode)] = ~(lc->generalReg[SourceAddress1(lc->opcode)]);
	update_flags(lc, DestinationAddress(lc->opcode));
}

void ST_op(vm* lc)
{
	memory_write(lc, lc->pcReg + sign_extend(lc->opcode & 0x01FF, 9), lc->generalReg[DestinationAddress(lc->opcode)]);
}

void STI_op(vm* lc)
{
	memory_write(lc, memory_read(lc, lc->pcReg + sign_extend(lc->opcode & 0x01FF, 9)), lc->generalReg[SourceAddress1(lc->opcode)]);
}

void STR_op(vm* lc)
{
	memory_write(lc, lc->generalReg[BaseRegAddress(lc->opcode)] + sign_extend(lc->opcode & 0x003F, 6), lc->generalReg[DestinationAddress(lc->opcode)]);
}


void TRAP_GETC(vm* lc)
{
	lc->generalReg[0x0] = (uint16_t)getchar();
}

void TRAP_OUT(vm* lc)
{
	putc((char)lc->generalReg[0x0], stdout);
	fflush(stdout);
}

void TRAP_PUTS(vm* lc)
{
	uint16_t* c = lc->memory + lc->generalReg[0x0];
	while (*c)
	{
		putc((char)*c, stdout);
		++c;
	}
	fflush(stdout);
}

void TRAP_IN(vm* lc)
{
	printf("Enter a character: ");
	char c = getchar();
	putc(c, stdout);
	lc->generalReg[0x0] = (uint16_t)c;
}

void TRAP_PUTSP(vm* lc)
{
	uint16_t* c = lc->memory + lc->generalReg[0x0];
	while (*c)
	{
		char char1 = (*c) & 0xFF;
		putc(char1, stdout);
		char char2 = (*c) >> 8;
		if (char2) putc(char2, stdout);
		++c;
	}
	fflush(stdout);
}

void TRAP_HALT(vm* lc)
{
	puts("HALT");
	fflush(stdout);
	exit(1);
}

void TRAP_ops(vm* lc, uint16_t trap_opcode)
{
	switch (trap_opcode)
	{
	case 0x20:
		TRAP_GETC(lc);
		break;
	case 0x21:
		TRAP_OUT(lc);
		break;
	case 0x22:
		TRAP_PUTS(lc);
		break;
	case 0x23:
		TRAP_IN(lc);
		break;
	case 0x24:
		TRAP_PUTSP(lc);
		break;
	case 0x25:
		TRAP_HALT(lc);
		break;
	default:
		printf("The trap is not found");
		exit(EXIT_FAILURE);
		break;
	}
}