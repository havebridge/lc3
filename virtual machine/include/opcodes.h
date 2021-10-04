#ifndef _OPCODES_H
#define _OPCODES_H

#include "vm.h"

void ADD_op(vm*);
void AND_op(vm*);
void BR_op(vm*);
void JMP_op(vm*);
void JSR_op(vm*);
void LD_op(vm*);
void LDI_op(vm*);
void LDR_op(vm*);
void LEA_op(vm*);
void NOT_op(vm*);
void ST_op(vm*);
void STI_op(vm*);
void STR_op(vm*);
void TRAP_ops(vm*, uint16_t);

#endif