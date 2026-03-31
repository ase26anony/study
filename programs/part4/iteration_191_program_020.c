/* reload_coverage.c
 * Designed to trigger GCC's reload pass push_reload function
 * with secondary reload initialization.
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
volatile int g_vol_int = 42;
volatile long long g_vol_ll = 0x123456789ABCDEFLL;
volatile float g_vol_float = 3.14159f;
volatile double g_vol_double = 2.718281828459045;

/* Vector types for additional reload complexity */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function with multiple inline asm statements requiring reloads */
static long long trigger_reloads(int a, long long b, float c, double d,
                                 v4si vec_int, v4sf vec_float) {
    long long result = 0;
    int out1, out2;
    long long out3;
    double out4;
    v4si out_vec;
    
    /* ASM 1: Mixed types with specific register constraints
     * Forces reloads due to mismatched constraints */
    asm volatile (
        /* Move int to output with specific constraint */
        "movl %1, %0\n\t"
        /* Add constant that might need secondary reload */
        "addl $0x7FFFFFFF, %0"
        : "=r" (out1)          /* Output in general reg */
        : "rm" (a)             /* Input: register or memory */
        : "cc"                 /* Clobbers flags */
    );
    result += out1;
    
    /* ASM 2: 64-bit operations with memory constraints
     * Likely to require secondary reloads for constants */
    asm volatile (
        /* Complex addressing mode */
        "movq %1, %%rax\n\t"
        "addq %2, %%rax\n\t"
        "movq %%rax, %0"
        : "=rm" (out3)         /* Output: reg or memory */
        : "rmi" (b),           /* Input 1: reg, mem, or immediate */
          "rmi" (0xFFFFFFFFFFFFFLL)  /* Large constant */
        : "rax", "cc"          /* Clobbers rax and flags */
    );
    result += out3;
    
    /* ASM 3: Floating point with integer conversion
     * Mixing modes often triggers reloads */
    asm volatile (
        /* Convert float to int */
        "cvttss2si %1, %0\n\t"
        /* Add with memory operand */
        "addl %2, %0"
        : "=r" (out2)          /* Output in general reg */
        : "xm" (c),            /* Input: SSE reg or memory */
          "m" (g_vol_int)      /* Memory operand */
        : "cc"
    );
    result += out2;
    
    /* ASM 4: Vector operations with specific constraints
     * Vector regs often need secondary reloads */
    asm volatile (
        /* Vector move and add */
        "movdqa %1, %0\n\t"
        "paddd %2, %0"
        : "=x" (out_vec)       /* Output in XMM reg */
        : "xm" (vec_int),      /* Input: XMM reg or memory */
          "xm" ((v4si){1, 2, 3, 4})  /* Constant vector */
        : /* No clobbers for vector regs on x86 */
    );
    
    /* Use vector result */
    for (int i = 0; i < 4; i++) {
        result += out_vec[i];
    }
    
    /* ASM 5: Double precision with specific constraints
     * Force memory addressing reloads */
    asm volatile (
        /* Load, modify, store */
        "movsd %1, %%xmm0\n\t"
        "addsd %2, %%xmm0\n\t"
        "movsd %%xmm0, %0"
        : "=m" (out4)          /* Output to memory */
        : "m" (d),             /* Input from memory */
          "m" (g_vol_double)   /* Global memory */
        : "xmm0", "cc"
    );
    
    /* Force use of output */
    result += (long long)out4;
    
    return result;
}

/* Wrapper to prevent inlining and create more reload context */
__attribute__((noinline))
static long long reload_wrapper(int x) {
    /* Initialize vectors */
    v4si vec_int = {x, x+1, x+2, x+3};
    v4sf vec_float = {x*1.0f, x*2.0f, x*3.0f, x*4.0f};
    
    /* Call with mixed arguments */
    return trigger_reloads(x, g_vol_ll + x,
                          g_vol_float * x,
                          g_vol_double / (x + 1),
                          vec_int, vec_float);
}

int main(int argc, char **argv) {
    /* Use argv to create variable input */
    int seed = 1;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Create varying inputs to prevent constant propagation */
    int base = (seed * 37) % 100;
    
    /* Trigger reloads multiple times */
    long long total = 0;
    for (int i = 0; i < 10; i++) {
        total += reload_wrapper(base + i);
    }
    
    printf("Result: %lld\n", total);
    return (int)(total % 256);
}
