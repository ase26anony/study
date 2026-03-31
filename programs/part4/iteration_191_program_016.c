/* reload_coverage.c
 * Designed to trigger specific uncovered lines in GCC's reload.cc
 * Compile with: gcc -O1 -fno-inline -fdump-rtl-reload -S reload_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
volatile int global_int = 42;
volatile long long global_ll = 0x123456789ABCDEFLL;
volatile float global_float = 3.14159f;
volatile double global_double = 2.718281828459045;

/* Function with complex inline assembly to trigger reloads */
static long long trigger_reloads(int a, long long b, float c, double d) {
    int out1, out2;
    long long out3;
    float out4;
    double out5;
    long long accumulator = 0;
    
    /* ASM 1: Mixed integer types with specific register constraints
     * Forces reloads due to register class mismatches */
    asm volatile (
        /* Template uses different sized operands */
        "movl %1, %0\n\t"
        "addl $1, %0"
        : "=r" (out1)          /* Output in general reg */
        : "irm" (a)            /* Input: immediate, reg, or memory */
        : "cc"                 /* Clobbers flags */
    );
    accumulator += out1;
    
    /* ASM 2: 64-bit operation with memory constraint
     * May require secondary reload for constant address */
    asm volatile (
        "movq %1, %0\n\t"
        "xorq $0xFF, %0"
        : "=r" (out3)          /* Output in 64-bit reg */
        : "m" (global_ll)      /* Input from memory - may need address reload */
        : "cc"
    );
    accumulator ^= out3;
    
    /* ASM 3: Floating point with integer conversion
     * Mixed modes trigger different inmode/outmode */
    asm volatile (
        "cvttss2si %1, %0"     /* Convert float to int */
        : "=r" (out2)          /* Integer output */
        : "x" (c)              /* SSE register input */
        : /* empty clobber */
    );
    accumulator += out2;
    
    /* ASM 4: Double precision with specific constraints
     * May require secondary reloads on some archs */
    asm volatile (
        "movsd %1, %0\n\t"
        "addsd %2, %0"
        : "=x" (out5)          /* Output in XMM register */
        : "xm" (d),            /* Input: XMM reg or memory */
          "xm" (global_double) /* Second input */
        : /* empty clobber */
    );
    /* Use the result to affect accumulator */
    accumulator += (long long)out5;
    
    /* ASM 5: Complex addressing mode with multiple clobbers
     * High register pressure forces more reloads */
    asm volatile (
        "leaq (%1,%2,1), %0\n\t"
        "addq $0x1234, %0"
        : "=&r" (out3)         /* Early clobber output */
        : "r" (&global_int),   /* Address may need reload */
          "r" (b & 0xFF)       /* Modified input */
        : "rax", "rcx", "cc"   /* Multiple clobbers increase pressure */
    );
    accumulator += out3;
    
    /* ASM 6: Vector-style operation (even without SIMD)
     * Using multiple outputs */
    asm volatile (
        "movl %2, %0\n\t"
        "movl %3, %1"
        : "=r" (out1), "=r" (out2)  /* Two outputs */
        : "irm" (a),                /* Different constraints */
          "irm" (global_int)        /* Global may need address reload */
        : "cc"
    );
    accumulator += out1 + out2;
    
    return accumulator;
}

int main(int argc, char **argv) {
    /* Use argv to create variable inputs (prevents constant propagation) */
    int int_val = argc > 1 ? atoi(argv[1]) : 100;
    long long ll_val = argc > 2 ? atoll(argv[2]) : 200;
    float float_val = argc > 3 ? atof(argv[3]) : 3.14f;
    double double_val = argc > 4 ? atof(argv[4]) : 2.71;
    
    /* Call the function multiple times with different args */
    long long result = 0;
    for (int i = 0; i < 3; i++) {
        result += trigger_reloads(
            int_val + i,
            ll_val + i,
            float_val + i,
            double_val + i
        );
    }
    
    printf("Result: %lld\n", result);
    return (int)(result & 0x7FFFFFFF);
}
