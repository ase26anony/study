#ifndef ERROR_ASM_H
#define ERROR_ASM_H

// Valid code to set parser state
inline void valid_func() {
    int x = 42;
    
    // Malformed asm - missing template string
    asm volatile ( : : : "memory" );
    
    // Another malformed asm in different context
    int y = asm;
}

// Macro to obfuscate
#define BAD_ASM_STMT asm volatile (
void another_func() {
    BAD_ASM_STMT : : : "memory" );
}

#endif
