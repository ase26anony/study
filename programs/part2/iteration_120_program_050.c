#ifndef ERROR_ASM_H
#define ERROR_ASM_H

// Valid code to get parser into function body context
inline void asm_error_test() {
    int x = 10;
    
    // Incomplete asm statement - missing template string
    // Should trigger: expected 'asm'
    asm volatile ( : : : "memory" );
    
    // Another invalid asm usage
    int y = asm;  // 'asm' used as value
}

// Macro that expands to incomplete asm
#define BAD_ASM_STMT asm volatile (

// Function using the bad macro
inline void macro_asm_error() {
    BAD_ASM_STMT : : : "memory");
}

#endif
