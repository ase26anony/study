#ifndef ERROR_ASM_H
#define ERROR_ASM_H

// Valid code to get parser into function context
inline void valid_func() {
    int x = 42;
}

// Malformed asm statement - missing template string
// Should trigger: expected 'asm'
inline void bad_asm1() {
    asm volatile ( : : : "memory" );
}

// asm in invalid context
inline void bad_asm2() {
    int x = asm;  // asm keyword used as value
}

// Macro that expands to incomplete asm
#define BAD_ASM_MACRO asm volatile (

inline void bad_asm3() {
    BAD_ASM_MACRO : : : "memory" );
}

#endif
