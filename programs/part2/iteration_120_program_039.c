#ifndef ERROR_ASM_H
#define ERROR_ASM_H

// Valid code to get parser into function body context
inline void asm_error_test() {
    int x = 10;
    
    // Missing assembly template string - should trigger "expected asm"
    asm volatile ( : : : "memory" );
    
    // Another invalid asm context
    int y = asm;
}

// Macro to obfuscate asm error
#define BAD_ASM_STMT asm volatile (
void macro_asm_error() {
    BAD_ASM_STMT : : : "memory");
}

#endif
