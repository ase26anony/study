#ifndef ERROR_ASM_H
#define ERROR_ASM_H

// Valid code to get parser into function body context
inline void test_asm_context() {
    int x = 42;
    
    // Invalid: asm without template string
    asm volatile ( : : : "memory" );
    
    // Another invalid asm usage
    int y = asm;
}

// Macro that expands to incomplete asm
#define BAD_ASM_1 asm volatile (
#define BAD_ASM_2 "mov %0, %1"

void use_bad_asm() {
    BAD_ASM_1 : "=r"(x) : "r"(y) );
}

#endif
