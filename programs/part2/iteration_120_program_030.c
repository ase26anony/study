#ifndef ERROR_ASM_H
#define ERROR_ASM_H

// Valid code to get parser into function body context
inline void valid_func() {
    int x = 42;
    
    // Malformed asm statement - missing template string
    // Should trigger: expected 'asm'
    asm volatile ( : : : "memory" );
    
    // Another malformed asm in different context
    int y = asm;  // 'asm' used as value
}

// Macro that expands to incomplete asm
#define BAD_ASM_STMT asm volatile (

// Function using the bad macro
inline void bad_asm_macro() {
    BAD_ASM_STMT : : : "memory");
}

#endif
