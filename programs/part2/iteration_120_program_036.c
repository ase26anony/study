#ifndef ERROR_ASM_H
#define ERROR_ASM_H

// RT_ASM: Incomplete GNU inline assembly
void asm_error1() {
    // Missing assembly template string
    asm volatile ( : : : "memory" );
}

// RT_ASM: 'asm' in invalid context
int asm_error2 = asm;

// RT_ASM: Using macro to hide the error
#define BAD_ASM asm volatile (
void asm_error3() {
    BAD_ASM : : : "memory");
}

#endif
