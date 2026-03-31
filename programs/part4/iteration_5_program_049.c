#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Dummy noinline function to force register saves/restores */
__attribute__((noinline)) 
int dummy_func(int a, int b, float c, double d, int e, int f) {
    volatile int result = a + b + (int)c + (int)d + e + f;
    return result;
}

/* External function to clobber registers */
extern void external_call(void);

int main(void) {
    /* Seed RNG for unpredictable values */
    srand(42);
    
    /* VOLATILE VARIABLES - prevent optimization */
    volatile int trigger = 100;
    volatile int accumulator = 0;
    
    /* MANY LIVE SCALAR VARIABLES (30+ variables) */
    /* Integer variables */
    int v1 = rand() % 100;
    int v2 = rand() % 100;
    int v3 = rand() % 100;
    int v4 = rand() % 100;
    int v5 = rand() % 100;
    int v6 = rand() % 100;
    int v7 = rand() % 100;
    int v8 = rand() % 100;
    int v9 = rand() % 100;
    int v10 = rand() % 100;
    int v11 = rand() % 100;
    int v12 = rand() % 100;
    int v13 = rand() % 100;
    int v14 = rand() % 100;
    int v15 = rand() % 100;
    
    /* Floating point variables */
    float f1 = (float)(rand() % 100) / 10.0f;
    float f2 = (float)(rand() % 100) / 10.0f;
    float f3 = (float)(rand() % 100) / 10.0f;
    float f4 = (float)(rand() % 100) / 10.0f;
    float f5 = (float)(rand() % 100) / 10.0f;
    float f6 = (float)(rand() % 100) / 10.0f;
    
    /* Double precision variables */
    double d1 = (double)(rand() % 100) / 10.0;
    double d2 = (double)(rand() % 100) / 10.0;
    double d3 = (double)(rand() % 100) / 10.0;
    double d4 = (double)(rand() % 100) / 10.0;
    double d5 = (double)(rand() % 100) / 10.0;
    
    /* EXPLICIT REGISTER VARIABLES with conflicting classes */
    /* Force binding to specific registers (compiler may honor these) */
    register int reg_ax asm("rax") = v1 + v2;
    register int reg_bx asm("rbx") = v3 + v4;
    register int reg_cx asm("rcx") = v5 + v6;
    
    /* Array for complex addressing modes */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = i * 2;
    }
    
    /* Pointer for non-offsettable addressing */
    int* ptr = array + 128;
    
    /* LOOP with invariant spilling */
    volatile int loop_limit = 50;
    for (int iteration = 0; iteration < loop_limit; iteration++) {
        /* COMPLEX INTERDEPENDENT EXPRESSIONS - extend live ranges */
        /* Mix integer and float operations */
        v1 = v2 + v3 * v4 - v5 / (v6 + 1);
        v2 = v7 ^ v8 | v9 & v10;
        v3 = (v11 << 2) | (v12 >> 3);
        
        /* Integer to float conversions (require register file moves) */
        f1 = (float)v1 + f2 * 3.14f;
        f2 = (float)(v2 ^ v3) / f3;
        
        /* Float to double conversions */
        d1 = (double)f1 + d2 * 2.71828;
        d2 = (double)(f2 + f3) / d3;
        
        /* Mixed size accesses in memory operations */
        /* Non-offsettable addressing: array[index + large_constant] */
        int index = v4 % 64;
        int val1 = array[index + 100];  /* Offset may be too large for some archs */
        int val2 = array[index + 150];  /* Another large offset */
        
        /* Use explicit register variables in conflicting contexts */
        /* These may need reloads if used in FP contexts */
        float temp_float = (float)reg_ax + f4;
        double temp_double = (double)reg_bx + d4;
        
        /* FUNCTION CALL - clobbers registers, forces save/restore */
        int call_result = dummy_func(v1, v2, f1, d1, v3, v4);
        
        /* INLINE ASSEMBLY with MANY CLOBBERS */
        /* Tell compiler many registers are unavailable */
        asm volatile (
            "nop\n\t"
            : 
            : 
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "xmm12", "xmm13", "xmm14", "xmm15", "memory"
        );
        
        /* MORE COMPLEX CALCULATIONS after registers clobbered */
        /* This forces reloads of previously computed values */
        v4 = v5 + v6 * call_result;
        v5 = v7 ^ v8 | (v9 & val1);
        
        /* Type conversions with memory accesses */
        short s1 = (short)(v10 % 65536);
        char c1 = (char)(v11 % 256);
        v12 = (int)s1 * (int)c1 + v13;
        
        /* Floating point with different precision */
        f3 = f4 * 2.0f + (float)v14;
        d3 = d4 / 3.0 + (double)v15;
        
        /* Complex pointer arithmetic */
        /* This often requires address reloads */
        int* complex_ptr = ptr + (v1 % 16) - (v2 % 8);
        int val3 = *complex_ptr;
        int* another_ptr = array + (v3 % 32) * 2;
        int val4 = *another_ptr;
        
        /* Bitwise and arithmetic combinations */
        v13 = (v14 & 0xFF) | ((v15 << 8) & 0xFF00);
        v14 = (v13 * v12) + (v11 ^ v10);
        v15 = (v14 >> 4) | (v13 << 4);
        
        /* Update volatile accumulator to prevent elimination */
        accumulator += v1 + v2 + v3 + (int)f1 + (int)d1 + val3 + val4;
        
        /* Mix explicit register variables into complex expressions */
        reg_ax = reg_bx + reg_cx * v4;
        reg_bx = reg_ax ^ v5;
        reg_cx = reg_bx | v6;
        
        /* Another function call to increase pressure */
        dummy_func(reg_ax, reg_bx, f3, d3, v7, v8);
        
        /* More inline asm with different clobbers */
        asm volatile (
            "nop\n\t"
            : 
            : 
            : "eax", "ebx", "ecx", "edx", "esi", "edi",
              "mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7",
              "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)",
              "memory"
        );
    }
    
    /* Final output to prevent dead code elimination */
    printf("Final accumulator: %d\n", accumulator);
    printf("Values: v1=%d, f1=%.2f, d1=%.2f\n", v1, f1, d1);
    
    return 0;
}
