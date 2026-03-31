/* reload_coverage.c
 * Designed to trigger specific uncovered lines in GCC's reload.cc
 * Compile with: gcc -O1 -fno-inline -fdump-rtl-reload -S reload_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
volatile int g_int = 42;
volatile long long g_llong = 0x123456789ABCDEFLL;
volatile float g_float = 3.14159f;
volatile double g_double = 2.718281828459045;

/* Function with multiple inline asm statements to trigger reloads */
static long long trigger_reloads(int a, long long b, float c, double d) {
    int out1, out2;
    long long out3;
    float out4;
    double out5;
    long long accumulator = 0;
    
    /* ASM 1: Mixed integer types with specific register constraints
     * This should trigger reloads with secondary reload info */
    asm volatile (
        "movl %1, %0\n\t"
        "addl $1, %0"
        : "=r" (out1)          /* Output in general reg */
        : "m" (g_int)          /* Input from memory - may need secondary reload */
        : "cc"                 /* Clobber flags */
    );
    accumulator += out1;
    
    /* ASM 2: 64-bit operation with mismatched constraints
     * Using 'i' constraint with non-immediate value may need secondary reload */
    asm volatile (
        "movq %1, %0\n\t"
        "addq $0x100, %0"
        : "=r" (out3)          /* Output in 64-bit reg */
        : "i" (b)              /* Input as immediate - may not fit, needs reload */
        : "cc"
    );
    accumulator += out3;
    
    /* ASM 3: Floating point with memory constraints
     * Mixing float/double with integer constraints causes reloads */
    asm volatile (
        "movss %1, %0\n\t"
        "addss %2, %0"
        : "=x" (out4)          /* Output in SSE register */
        : "m" (g_float),       /* Input from memory */
          "x" (c)              /* Input in SSE register */
        : 
    );
    /* Convert float to int for accumulator */
    int temp = (int)out4;
    accumulator += temp;
    
    /* ASM 4: Complex addressing mode with multiple constraints
     * This often requires secondary reloads for address computation */
    asm volatile (
        "movsd %1, %0\n\t"
        "mulsd %2, %0"
        : "=x" (out5)          /* Output in XMM register */
        : "m" (*(const double(*)[1])&g_double),  /* Complex memory address */
          "x" (d)              /* Input in XMM register */
        : 
    );
    temp = (int)out5;
    accumulator += temp;
    
    /* ASM 5: Multiple outputs with specific register classes
     * Using explicit register constraints to force reloads */
    asm volatile (
        "movl %2, %0\n\t"
        "movl %2, %1\n\t"
        "addl $10, %0\n\t"
        "subl $5, %1"
        : "=&r" (out1), "=&r" (out2)  /* Two earlyclobber outputs */
        : "mr" (a)                     /* Input in memory or register */
        : "cc"
    );
    accumulator += out1 + out2;
    
    /* ASM 6: Using vector extensions if available
     * This creates different machine modes */
#ifdef __SSE2__
    typedef long long v2ll __attribute__((vector_size(16)));
    v2ll vec_in = {b, b + 1};
    v2ll vec_out;
    
    asm volatile (
        "paddq %1, %0"
        : "=x" (vec_out)
        : "x" (vec_in), "0" (vec_in)
        : 
    );
    accumulator += vec_out[0] + vec_out[1];
#endif
    
    return accumulator;
}

int main(int argc, char *argv[]) {
    /* Use argv to prevent constant propagation */
    int base = (argc > 1) ? atoi(argv[1]) : 100;
    
    /* Initialize variables with non-constant values */
    int int_val = base + g_int;
    long long llong_val = g_llong + base;
    float float_val = g_float + (float)base;
    double double_val = g_double + (double)base;
    
    /* Call function that triggers reloads */
    long long result = trigger_reloads(int_val, llong_val, float_val, double_val);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %lld\n", result);
    
    return (int)(result % 1000);
}
