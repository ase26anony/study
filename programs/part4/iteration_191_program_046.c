/* reload_test.c - Test program to trigger push_reload with secondary reloads */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
volatile int g_int = 42;
volatile long long g_llong = 0x123456789ABCDEF0LL;
volatile float g_float = 3.14159f;
volatile double g_double = 2.718281828459045;

/* Vector type for testing vector reloads */
typedef int v4si __attribute__((vector_size(16)));

/* Function that will trigger various reload scenarios */
static long long trigger_reloads(int a, long long b, float c, double d, v4si v) {
    int out_int1, out_int2;
    long long out_llong;
    float out_float;
    double out_double;
    v4si out_vec;
    long long accumulator = 0;
    
    /* Force values to be used to prevent optimization */
    volatile int use_a = a;
    volatile long long use_b = b;
    (void)use_a; (void)use_b;
    
    /* 
     * ASM 1: Mixed integer types with specific register constraints
     * This should trigger reloads due to register pressure and constraints
     */
    asm volatile (
        /* Move with potential need for secondary reload */
        "movl %1, %0\n\t"
        /* Additional operation to use more registers */
        "addl $1, %0"
        : "=&r" (out_int1)      /* Early clobber output, general reg constraint */
        : "irm" (g_int)         /* Input: immediate, register, or memory */
        : "cc"                  /* Clobbers condition codes */
    );
    accumulator += out_int1;
    
    /* 
     * ASM 2: 64-bit operations with mismatched constraints
     * Using 'A' constraint (eax/edx pair) on x86
     */
    asm volatile (
        "movq %1, %0\n\t"
        "addq $0x1000, %0"
        : "=A" (out_llong)      /* Output in eax:edx pair */
        : "m" (g_llong)         /* Memory input */
        : "cc"
    );
    accumulator += out_llong;
    
    /* 
     * ASM 3: Floating point with general register constraints
     * This often requires secondary reloads on some architectures
     */
    asm volatile (
        "movd %1, %0\n\t"       /* Move float through integer register */
        : "=r" (out_int2)       /* Output in general register */
        : "x" (c)               /* Input in SSE/vector register */
        : /* No clobbers */
    );
    accumulator += out_int2;
    
    /* 
     * ASM 4: Double with memory constraint and specific register
     * Using 'f' constraint for floating point register
     */
    asm volatile (
        "movsd %1, %0"
        : "=f" (out_double)     /* Output in FP register */
        : "m" (d)               /* Memory input */
        : /* No clobbers */
    );
    /* Convert double to int for accumulator */
    accumulator += (long long)out_double;
    
    /* 
     * ASM 5: Vector operations with memory addressing
     * Using 'x' constraint for vector registers
     */
    asm volatile (
        "movdqa %1, %0\n\t"
        "paddd %2, %0"
        : "=x" (out_vec)        /* Output in vector register */
        : "xm" (v),             /* Vector input (register or memory) */
          "xm" (v)              /* Same vector again */
        : /* No clobbers */
    );
    /* Sum vector elements */
    for (int i = 0; i < 4; i++) {
        accumulator += out_vec[i];
    }
    
    /* 
     * ASM 6: Complex addressing mode that might need secondary reload
     * Using multiple constraints to force reload decisions
     */
    int base = 100;
    int index = a;
    int scale = 4;
    int result;
    
    asm volatile (
        "leal (%1, %2, %3), %0"
        : "=r" (result)
        : "r" (base),           /* Base in register */
          "r" (index),          /* Index in register */
          "i" (scale)           /* Scale as immediate */
        : "cc"
    );
    accumulator += result;
    
    /* 
     * ASM 7: Inline asm with output in memory but input requiring computation
     * This can trigger output reloads
     */
    int mem_output;
    asm volatile (
        "movl %1, %0\n\t"
        : "=m" (mem_output)     /* Output to memory */
        : "ri" (a * 2 + 1)      /* Input: register or immediate requiring computation */
        : "cc"
    );
    accumulator += mem_output;
    
    return accumulator;
}

int main(int argc, char *argv[]) {
    /* Use argv to create variable values to prevent constant propagation */
    int int_val = argc > 1 ? atoi(argv[1]) : 12345;
    long long llong_val = argc > 2 ? atoll(argv[2]) : 6789012345LL;
    float float_val = argc > 3 ? atof(argv[3]) : 1.234f;
    double double_val = argc > 4 ? atof(argv[4]) : 5.678;
    
    /* Initialize vector */
    v4si vec_val = { int_val, int_val + 1, int_val + 2, int_val + 3 };
    
    /* Call function multiple times with different values */
    long long result1 = trigger_reloads(int_val, llong_val, float_val, double_val, vec_val);
    long long result2 = trigger_reloads(int_val + 1, llong_val + 1, 
                                       float_val + 1.0f, double_val + 1.0, vec_val);
    
    /* Use results to prevent dead code elimination */
    printf("Result 1: %lld\n", result1);
    printf("Result 2: %lld\n", result2);
    printf("Total: %lld\n", result1 + result2);
    
    return (int)((result1 + result2) & 0x7FFFFFFF);
}
