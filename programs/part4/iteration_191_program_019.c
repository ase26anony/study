/* reload_coverage.c - Test program to cover GCC reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
volatile int global_int = 12345;
volatile long long global_ll = 0x123456789ABCDEFLL;
volatile float global_float = 3.14159f;
volatile double global_double = 2.718281828459045;

/* Vector types for testing different machine modes */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function with multiple inline asm statements to trigger reloads */
static long long trigger_reloads(int a, long long b, float c, double d,
                                 v4si vec_int, v4sf vec_float) {
    long long accumulator = 0;
    int out_int1, out_int2;
    long long out_ll1, out_ll2;
    float out_float;
    double out_double;
    v4si out_vec_int;
    v4sf out_vec_float;
    
    /* ASM 1: Mixed types with specific register constraints */
    /* This should trigger reloads due to register class mismatches */
    asm volatile (
        /* Move int to output with 'r' constraint, but input might need reload */
        "movl %1, %0\n\t"
        /* Use the value in a computation */
        "addl $100, %0"
        : "=r" (out_int1)          /* Output in general register */
        : "irm" (a)                /* Input: immediate, register, or memory */
        : "cc"                     /* Clobber condition codes */
    );
    accumulator += out_int1;
    
    /* ASM 2: Floating point with general register constraints */
    /* This may require secondary reloads on some architectures */
    asm volatile (
        /* Simulate moving float through integer register */
        "movd %1, %0\n\t"
        "psrldq $8, %0"            /* Shift right (x86-specific) */
        : "=r" (out_int2)          /* Output in general register */
        : "x" (c)                  /* Input in SSE register */
        : "cc"
    );
    accumulator += out_int2;
    
    /* ASM 3: 64-bit operations with memory constraints */
    /* Likely to trigger address reloads */
    asm volatile (
        /* Load from memory, operate, store back */
        "movq %1, %%rax\n\t"
        "addq %2, %%rax\n\t"
        "movq %%rax, %0"
        : "=rm" (out_ll1)          /* Output: register or memory */
        : "mr" (b),                /* Input: memory or register */
          "irm" (global_ll)        /* Input: immediate, register, or memory */
        : "rax", "cc"              /* Clobber rax and condition codes */
    );
    accumulator += out_ll1;
    
    /* ASM 4: Vector operations with specific constraints */
    /* May require secondary reloads for vector registers */
    asm volatile (
        /* Vector move with possible reload */
        "movdqa %1, %0\n\t"
        "paddd %2, %0"             /* Packed add (x86) */
        : "=x" (out_vec_int)       /* Output in SSE register */
        : "xm" (vec_int),          /* Input: SSE register or memory */
          "xm" (vec_int)           /* Same vector again */
        : "cc"
    );
    
    /* Use vector result to prevent elimination */
    for (int i = 0; i < 4; i++) {
        accumulator += out_vec_int[i];
    }
    
    /* ASM 5: Mixed float/int with multiple outputs */
    /* Complex constraints to stress reload logic */
    asm volatile (
        /* Multiple operations in one template */
        "cvtsd2si %2, %0\n\t"      /* Convert double to int */
        "movq %3, %1\n\t"          /* Move long long */
        "addq $1, %1"
        : "=r" (out_int1),         /* Output 1: general register */
          "=rm" (out_ll2)          /* Output 2: register or memory */
        : "x" (d),                 /* Input 1: SSE register (double) */
          "irm" (accumulator),     /* Input 2: immediate, register, or memory */
          "m" (global_double)      /* Input 3: memory (double) */
        : "cc"
    );
    accumulator = out_int1 + out_ll2;
    
    /* ASM 6: Force address computation reload */
    /* Using 'p' constraint (address) can trigger interesting reloads */
    {
        int array[4] = {1, 2, 3, 4};
        int index = a & 3;
        
        asm volatile (
            /* Load through computed address */
            "movl (%1, %2, 4), %0"
            : "=r" (out_int2)
            : "r" (array),         /* Base address */
              "r" (index)          /* Scaled index */
            : "memory"
        );
        accumulator += out_int2;
    }
    
    /* ASM 7: Try to trigger secondary reload with constant pool */
    {
        /* Force constant pool reference */
        static const double big_const = 1.234567890123456e100;
        
        asm volatile (
            /* Load constant through register */
            "movsd %1, %0"
            : "=x" (out_double)
            : "m" (big_const)      /* Memory constraint on constant */
            : /* no clobbers */
        );
        
        /* Use the result */
        accumulator += (long long)out_double;
    }
    
    return accumulator;
}

/* Main function with varied inputs */
int main(int argc, char *argv[]) {
    /* Initialize with non-constant values */
    int int_val = (argc > 1) ? atoi(argv[1]) : 42;
    long long ll_val = (argc > 2) ? atoll(argv[2]) : 0x987654321LL;
    float float_val = (argc > 3) ? atof(argv[3]) : 1.414213f;
    double double_val = (argc > 4) ? atof(argv[4]) : 3.1415926535;
    
    /* Initialize vectors */
    v4si vec_int = {int_val, int_val + 1, int_val + 2, int_val + 3};
    v4sf vec_float = {float_val, float_val * 2, float_val * 3, float_val * 4};
    
    /* Call function multiple times with different arguments */
    long long result = 0;
    for (int i = 0; i < 3; i++) {
        result += trigger_reloads(int_val + i, 
                                 ll_val + i, 
                                 float_val + i,
                                 double_val + i,
                                 vec_int,
                                 vec_float);
    }
    
    /* Use global variables to prevent optimization */
    result += global_int + global_ll;
    
    printf("Result: %lld\n", result);
    return (int)(result & 0x7FFFFFFF);
}
