/* reload_test.c - Test program to trigger GCC reload pass uncovered lines */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile int g_int = 12345;
volatile long long g_llong = 9876543210LL;
volatile float g_float = 3.14159f;
volatile double g_double = 2.718281828459045;
volatile int g_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

/* Vector types for SIMD reloads */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function to trigger various reload scenarios */
static long long trigger_reloads(int a, long long b, float c, double d, 
                                 v4si vec_int, v4sf vec_float) {
    long long accumulator = 0;
    int out_int1, out_int2;
    long long out_llong;
    float out_float;
    double out_double;
    v4si out_vec_int;
    v4sf out_vec_float;
    
    /* Force use of arguments to prevent optimization */
    volatile int dummy = a;
    (void)dummy;
    
    /* 
     * Test Case 1: Mixed types with specific register constraints
     * This forces reloads due to register class mismatches
     */
    asm volatile (
        /* Move int to output with 'r' constraint - may need secondary reload */
        "movl %1, %0\n\t"
        /* Use the value in a computation */
        "addl $100, %0"
        : "=r" (out_int1)          /* Output in general register */
        : "i" (g_int)              /* Input as immediate (may need reload) */
        : "cc"                     /* Clobber condition codes */
    );
    accumulator += out_int1;
    
    /*
     * Test Case 2: Memory operand with complex addressing
     * May require secondary reload for address computation
     */
    asm volatile (
        /* Load from memory with index */
        "movq (%1,%2,4), %0"
        : "=r" (out_llong)         /* Output in register */
        : "r" (g_array),           /* Base address in register */
          "r" (a & 7)              /* Index in register */
        : "memory"
    );
    accumulator += out_llong;
    
    /*
     * Test Case 3: Floating point with integer conversion
     * Forces different modes and potential secondary reloads
     */
    asm volatile (
        /* Convert float to int */
        "cvttss2si %1, %0\n\t"
        /* Use in computation */
        "imull $2, %0, %0"
        : "=r" (out_int2)          /* Integer output */
        : "x" (c)                  /* Float input in SSE register */
        : "cc"
    );
    accumulator += out_int2;
    
    /*
     * Test Case 4: Double precision with memory constraint
     * May require reload due to memory address computation
     */
    asm volatile (
        /* Load double, add, store back */
        "movsd %1, %%xmm0\n\t"
        "addsd %2, %%xmm0\n\t"
        "movsd %%xmm0, %0"
        : "=m" (out_double)        /* Memory output */
        : "m" (g_double),          /* Memory input */
          "m" (d)                  /* Another memory input */
        : "xmm0", "cc"
    );
    /* Use the result */
    accumulator += (long long)out_double;
    
    /*
     * Test Case 5: Vector operations with specific constraints
     * Vector moves often require special handling
     */
    asm volatile (
        /* Vector move with possible reload */
        "movdqa %1, %0\n\t"
        /* Add constant vector */
        "paddd %2, %0"
        : "=x" (out_vec_int)       /* XMM register output */
        : "x" (vec_int),           /* XMM register input */
          "m" (vec_int)            /* Same data from memory - forces choice */
        : "cc"
    );
    /* Extract element from vector */
    accumulator += out_vec_int[0];
    
    /*
     * Test Case 6: Multiple outputs with different constraints
     * Complex case that stresses reload logic
     */
    asm volatile (
        /* Multiple operations in one asm */
        "movd %1, %%xmm0\n\t"      /* Move float to vector reg */
        "cvtsi2sd %2, %%xmm1\n\t"  /* Convert int to double */
        "movd %%xmm0, %0\n\t"      /* Move back to float */
        "movq %%xmm1, %3"          /* Move double to output */
        : "=r" (out_float),        /* Float output in general reg (needs reload) */
          "=m" (out_vec_float)     /* Vector output to memory */
        : "r" (out_int1),          /* Integer input */
          "x" (g_float)            /* Float input in vector reg */
        : "xmm0", "xmm1", "cc", "memory"
    );
    accumulator += (long long)out_float;
    
    /*
     * Test Case 7: Inline asm with 'a' constraint (accumulator)
     * Forces specific register allocation
     */
    asm volatile (
        /* Use accumulator explicitly */
        "movl %1, %%eax\n\t"
        "imull %%eax, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (out_int1)          /* Output */
        : "a" (a)                  /* Input must be in eax */
        : "cc"
    );
    accumulator += out_int1;
    
    /*
     * Test Case 8: Large immediate that may need reload
     */
    asm volatile (
        /* Load large constant */
        "movabsq $0x123456789ABCDEF0, %0\n\t"
        "addq %1, %0"
        : "=r" (out_llong)         /* Output */
        : "r" (accumulator)        /* Input */
        : "cc"
    );
    accumulator = out_llong;
    
    return accumulator;
}

/* Main function with varied inputs */
int main(int argc, char *argv[]) {
    /* Initialize with argv to prevent constant propagation */
    int base = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Create varied inputs */
    int int_val = base + g_int;
    long long llong_val = (long long)base * g_llong;
    float float_val = (float)base * g_float;
    double double_val = (double)base * g_double;
    
    /* Vector inputs */
    v4si vec_int = {base, base + 1, base + 2, base + 3};
    v4sf vec_float = {(float)base, (float)base + 1.0f, 
                      (float)base + 2.0f, (float)base + 3.0f};
    
    /* Call function multiple times with different arguments */
    long long result1 = trigger_reloads(int_val, llong_val, 
                                       float_val, double_val,
                                       vec_int, vec_float);
    
    long long result2 = trigger_reloads(int_val + 1, llong_val + 1,
                                       float_val + 1.0f, double_val + 1.0,
                                       vec_int, vec_float);
    
    /* Use results to prevent optimization */
    printf("Result 1: %lld\n", result1);
    printf("Result 2: %lld\n", result2);
    printf("Difference: %lld\n", result2 - result1);
    
    return (result1 != result2) ? 0 : 1;
}
