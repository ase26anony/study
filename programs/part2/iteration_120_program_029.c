#ifndef ERROR_ASM_H
#define ERROR_ASM_H

// Valid code to get parser into function context
inline void asm_test_function() {
    int x = 42;
    
    // Malformed asm statement - missing template string
    // Should trigger: expected 'asm'
    asm volatile ( : : : "memory" );
    
    // Another invalid asm context
    int y = asm;
}

// Macro to obfuscate
#define BAD_ASM_STMT asm volatile (

void another_func() {
    // Expanded macro creates malformed asm
    BAD_ASM_STMT : : : "memory");
}

#endif
