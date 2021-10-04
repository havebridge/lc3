#include "include/vm.h"
#include "include/opcodes.h"

DWORD fdwMode, fdwOldMode;
HANDLE hStdin = INVALID_HANDLE_VALUE;

void disable_input_buffering()
{
	hStdin = GetStdHandle(STD_INPUT_HANDLE);
	GetConsoleMode(hStdin, &fdwOldMode); /* save old mode */
	fdwMode = fdwOldMode
		^ ENABLE_ECHO_INPUT  /* no input echo */
		^ ENABLE_LINE_INPUT; /* return when one or
								more characters are available */
	SetConsoleMode(hStdin, fdwMode); /* set new mode */
	FlushConsoleInputBuffer(hStdin); /* clear buffer */
}

void restore_input_buffering()
{
	SetConsoleMode(hStdin, fdwOldMode);
}

void handle_interrupt(int signal)
{
	restore_input_buffering();
	printf("\n");
	exit(-2);
}

static uint16_t check_key()
{
	return WaitForSingleObject(hStdin, 1000) == WAIT_OBJECT_0 && _kbhit();
}


uint16_t sign_extend(uint16_t x, int bit_count)
{
	if ((x >> (bit_count - 1)) & 1) {
		x |= (0xFFFF << bit_count);
	}
	return x;
}

static uint16_t swap16(uint16_t x)
{
	return (x << 8) | (x >> 8);
}


void memory_write(vm* lc, uint16_t address, uint16_t value)
{
	lc->memory[address] = value;
}

uint16_t memory_read(vm* lc, uint16_t address)
{
	if (address == KBSR)
	{
		if (check_key())
		{
			lc->memory[KBSR] = (1 << 15);
			lc->memory[KBDR] = getchar();
		}
		else
		{
			lc->memory[KBSR] = 0;
		}
	}
	return lc->memory[address];
}


void update_flags(vm* lc, uint16_t reg)
{
	if (lc->generalReg[reg] == 0)
	{
		lc->condReg = COND_FLAG_ZERO;
	}
	else if ((lc->generalReg[reg] >> 15) == 1)
	{
		lc->condReg = COND_FLAG_NEG;
	}
	else
	{
		lc->condReg = COND_FLAG_POS;
	}
}

void vm_init(vm* lc)
{
	lc->pcReg = 0x3000;
	lc->condReg = 0;
	lc->opcode = 0;

	memset(lc->memory, 0, sizeof(lc->memory));
	memset(lc->generalReg, 0, sizeof(lc->generalReg));
}

void vm_load_image(vm* lc, const char* path)
{
	FILE* image = fopen(path, "rb");
	if (!image)
	{
		perror("Failed to open file: ");
		exit(EXIT_FAILURE);
	}

	uint16_t address;
	fread(&address, sizeof(uint16_t), 1, image);
	address = swap16(address);

	size_t size = UINT16_MAX - address;
	uint16_t* p = lc->memory + address;
	size_t result = fread(p, sizeof(uint16_t), (size_t)size, image);

	while (result-- > 0)
	{
		*p = swap16(*p);
		p++;
	}

	fclose(image);
}

void vm_run(vm* lc)
{
	lc->opcode = memory_read(lc, lc->pcReg++);
	uint16_t instr = lc->opcode >> 12;

	switch (instr)
	{
	case 0x0:
		BR_op(lc);
		break;
	case 0x1:
		ADD_op(lc);
		break;
	case 0x2:
		LD_op(lc);
		break;
	case 0x3:
		ST_op(lc);
		break;
	case 0x4:
		JSR_op(lc);
		break;
	case 0x5:
		AND_op(lc);
		break;
	case 0x6:
		LDR_op(lc);
		break;
	case 0x7:
		STR_op(lc);
		break;
	case 0x8:
		break;
	case 0x9:
		NOT_op(lc);
		break;
	case 0xA:
		LDI_op(lc);
		break;
	case 0xB:
		STI_op(lc);
		break;
	case 0xC:
		JMP_op(lc);
		break;
	case 0xD:
		break;
	case 0xE:
		LEA_op(lc);
		break;
	case 0xF:
		TRAP_ops(lc, lc->opcode & 0xFF);
		break;
	default:
		printf("The instruction is not found\n");
		exit(EXIT_FAILURE);
		break;
	}
}