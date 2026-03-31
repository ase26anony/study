/* reload_coverage.c
 * Designed to trigger GCC's reload pass push_reload function
 * with secondary reload initialization
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variables to prevent optimization */
volatile int g_volatile_int = 12345;
volatile long long g_volatile_ll = 9876543210LL;
volatile float g_volatile_float = 3.14159f;
volatile double g_volatile_double = 2.718281828459045;

/* Vector types for additional mode variety */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function to trigger various reload scenarios */
static long long trigger_reloads(int a, long long b, float c, double d, 
                                 v4si vec_int, v4sf vec_float) {
    long long accumulator = 0;
    int out_int1, out_int2;
    long long out_ll1, out_ll2;
    float out_float;
    double out_double;
    v4si out_vec_int;
    v4sf out_vec_float;
    
    /* ASM 1: Mixed modes with specific register constraints
     * This forces reloads due to register class mismatches */
    asm volatile (
        /* Move int to output with 'r' constraint - may need secondary reload */
        "movl %1, %0\n\t"
        /* Use the value in a computation */
        "addl $100, %0"
        : "=r" (out_int1)          /* Output in general reg */
        : "i" (g_volatile_int)     /* Immediate that may need reload */
        : "cc"                     /* Clobber flags */
    );
    accumulator += out_int1;
    
    /* ASM 2: Floating point with memory constraints
     * May trigger secondary reloads for memory addresses */
    asm volatile (
        /* Load float, do operation, store */
        "movss %1, %%xmm0\n\t"
        "mulss %2, %%xmm0\n\t"
        "movss %%xmm0, %0"
        : "=m" (out_float)         /* Memory output */
        : "m" (c),                 /* Memory input */
          "m" (g_volatile_float)   /* Global memory input */
        : "xmm0", "cc"            /* Clobber xmm0 and flags */
    );
    accumulator += (long long)out_float;
    
    /* ASM 3: 64-bit operations with specific constraints
     * Using 'a' constraint for accumulator register */
    asm volatile (
        /* Complex addressing mode that may need reload */
        "movq %1, %%rax\n\t"
        "addq %2, %%rax\n\t"
        "movq %%rax, %0"
        : "=r" (out_ll1)           /* Output in any general reg */
        : "r" (b),                 /* Input in register */
          "m" (g_volatile_ll)      /* Memory operand */
        : "rax", "cc"             /* Clobber rax and flags */
    );
    accumulator += out_ll1;
    
    /* ASM 4: Vector operations with mismatched constraints
     * Vector to scalar transfer may need special handling */
    asm volatile (
        /* Extract element from vector */
        "movd %1, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (out_int2)          /* Integer output */
        : "x" (vec_int)            /* Vector in SSE register */
        : "rax", "cc"             /* Clobber rax and flags */
    );
    accumulator += out_int2;
    
    /* ASM 5: Double precision with register pressure
     * Force use of specific registers */
    register double d1 asm("xmm1") = d;
    register double d2 asm("xmm2") = g_volatile_double;
    asm volatile (
        /* Double precision operation */
        "movsd %1, %%xmm0\n\t"
        "addsd %2, %%xmm0\n\t"
        "movsd %%xmm0, %0"
        : "=m" (out_double)        /* Memory output */
        : "x" (d1),                /* Input in xmm1 */
          "x" (d2)                 /* Input in xmm2 */
        : "xmm0", "cc"            /* Clobber xmm0 and flags */
    );
    accumulator += (long long)out_double;
    
    /* ASM 6: Multiple outputs with complex constraints
     * This creates pressure on register allocator */
    asm volatile (
        /* Multiple operations in one asm */
        "movq %2, %%rax\n\t"
        "addq %%rax, %%rax\n\t"
        "movq %%rax, %0\n\t"
        "movl %3, %%ebx\n\t"
        "imull $2, %%ebx\n\t"
        "movl %%ebx, %1"
        : "=r" (out_ll2),          /* First output */
          "=r" (out_int1)          /* Second output */
        : "r" (b),                 /* Input 1 */
          "m" (g_volatile_int),    /* Input 2 - memory */
          "r" (a)                  /* Input 3 */
        : "rax", "rbx", "cc"      /* Clobber multiple regs */
    );
    accumulator += out_ll2 + out_int1;
    
    /* ASM 7: Vector copy with alignment constraints
     * May trigger reloads for aligned moves */
    asm volatile (
        "movaps %1, %0"
        : "=x" (out_vec_float)     /* SSE register output */
        : "x" (vec_float)          /* SSE register input */
        : /* No clobbers for simple move */
    );
    
    /* Use vector result to affect accumulator */
    float vec_sum = out_vec_float[0] + out_vec_float[1] + 
                    out_vec_float[2] + out_vec_float[3];
    accumulator += (long long)vec_sum;
    
    return accumulator;
}

int main(int argc, char *argv[]) {
    /* Use argv to create variable values to prevent constant propagation */
    int base = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Initialize variables with non-constant values */
    int int_val = base + 1;
    long long ll_val = (long long)base * 1000LL;
    float float_val = (float)base / 3.0f;
    double double_val = (double)base * 1.234567;
    
    /* Initialize vectors */
    v4si vec_int = {base, base + 1, base + 2, base + 3};
    v4sf vec_float = {float_val, float_val * 2.0f, 
                      float_val * 3.0f, float_val * 4.0f};
    
    /* Call function multiple times to increase reload opportunities */
    long long result = 0;
    for (int i = 0; i < 3; i++) {
        result += trigger_reloads(int_val + i, ll_val + i, 
                                 float_val + i, double_val + i,
                                 vec_int, vec_float);
    }
    
    printf("Result: %lld\n", result);
    
    /* Use result to prevent dead code elimination */
    if (result > 1000000) {
        printf("Large result detected\n");
    }
    
    return (int)(result % 1000);
}
