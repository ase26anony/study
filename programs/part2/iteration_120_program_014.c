#ifndef ERROR_ASM_H
#define ERROR_ASM_H

// Valid code to get parser into function body context
inline void test_asm_context() {
    int x = 42;
    
    // ERROR: Incomplete asm statement - missing template string
    // Should trigger RT_ASM diagnostic
    asm volatile ( : : : "memory" );
    
    // Another error: asm in invalid context
    int y = asm;
}

// Macro that expands to incomplete asm
#define BAD_ASM asm volatile (
void use_bad_asm() {
    BAD_ASM : : : "memory");
}

#endif
