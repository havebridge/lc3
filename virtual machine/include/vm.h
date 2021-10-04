#ifndef _VM_H
#define _VM_H

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <Windows.h>

#define COND_FLAG_NEG 1 << 2
#define COND_FLAG_ZERO 1 << 1
#define COND_FLAG_POS 1 << 0

enum
{
	KBSR = 0xFE00,
	KBDR = 0xFE02
};

typedef struct
{
	uint16_t memory[UINT16_MAX];

	uint16_t pcReg;
	uint16_t condReg;
	uint16_t generalReg[0x8];

	uint16_t opcode;
}vm;

void disable_input_buffering(void);
void restore_input_buffering(void);
void handle_interrupt(int);

uint16_t sign_extend(uint16_t, int);

void memory_write(vm*, uint16_t, uint16_t);
uint16_t memory_read(vm*, uint16_t);
void update_flags(vm*, uint16_t);

void vm_init(vm*);
void vm_load_image(vm*, const char*);
void vm_run(vm*);

#endif