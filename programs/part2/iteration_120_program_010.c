#ifndef ERROR_ASM_H
#define ERROR_ASM_H

// Valid code to get parser into function body context
inline void asm_test_function() {
    int x = 42;
    
    // Malformed asm statement - missing template string
    // Should trigger RT_ASM diagnostic
    asm volatile ( : : : "memory" );
    
    // Another invalid asm usage
    int y = asm;
}

// Macro to obfuscate asm error
#define BAD_INLINE_ASM asm volatile (
void bad_asm_via_macro() {
    BAD_INLINE_ASM : : : "memory");
}

#endif
