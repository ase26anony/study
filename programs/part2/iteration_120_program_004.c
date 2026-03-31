#ifndef ERROR_ASM_H
#define ERROR_ASM_H

// Valid code to get parser into function body context
inline void test_asm_errors() {
    int x = 0;
    
    // Missing assembly template string
    asm volatile ( : : : "memory" );
    
    // asm in invalid context
    int y = asm;
    
    // Incomplete asm statement
    asm ("mov %0, %1" : : );
}

// Macro to obfuscate
#define BAD_ASM_1 asm volatile (
#define BAD_ASM_2 "mov eax, ebx"

void use_bad_asm() {
    BAD_ASM_1 BAD_ASM_2 : : : "memory");
}

#endif
