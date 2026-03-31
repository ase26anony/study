/* Test program to trigger reload.cc push_reload uncovered lines */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile int g_int = 42;
volatile long long g_llong = 9876543210LL;
volatile float g_float = 3.14159f;
volatile double g_double = 2.718281828459045;
volatile int g_array[10] = {0,1,2,3,4,5,6,7,8,9};

/* Function with complex inline assembly to trigger reloads */
static long long trigger_reloads(int a, long long b, float c, double d, int *ptr)
{
    long long accumulator = 0;
    int out_int1, out_int2;
    long long out_llong;
    float out_float;
    double out_double;
    int temp;
    
    /* ASM 1: Mixed types with specific register constraints */
    /* This should trigger reloads with secondary reload info */
    asm volatile (
        /* Move int to output with specific constraint */
        "movl %1, %0\n\t"
        /* Use the value in computation */
        "addl $100, %0"
        : "=&r" (out_int1)        /* Early clobber output */
        : "irm" (a)               /* Input: int in reg, memory, or immediate */
        : "cc"                    /* Clobber flags */
    );
    accumulator += out_int1;
    
    /* ASM 2: Floating point with general reg constraints */
    /* May require secondary reloads for float->int conversion */
    asm volatile (
        /* Simulate some operation */
        "movd %1, %%xmm0\n\t"
        "cvttss2si %%xmm0, %0"
        : "=r" (temp)             /* Output in general reg */
        : "x" (c)                 /* Input in SSE register */
        : "xmm0", "cc"
    );
    accumulator += temp;
    
    /* ASM 3: Memory addressing with complex constraints */
    /* Forces address reloads with possible secondary reloads */
    asm volatile (
        /* Load from memory address with offset */
        "movl (%1), %0\n\t"
        "addl %2, %0"
        : "=&r" (out_int2)        /* Early clobber */
        : "r" (&g_array[3]),      /* Address in register */
          "irm" (g_int)           /* Can be reg, mem, or immediate */
        : "memory", "cc"
    );
    accumulator += out_int2;
    
    /* ASM 4: 64-bit operations with specific constraints */
    /* May trigger different mode reloads */
    asm volatile (
        /* 64-bit move and operation */
        "movq %1, %0\n\t"
        "addq %2, %0"
        : "=&r" (out_llong)       /* Early clobber 64-bit output */
        : "rm" (b),               /* 64-bit input in reg/memory */
          "irm" (1000LL)          /* 64-bit constant */
        : "cc"
    );
    accumulator += out_llong;
    
    /* ASM 5: Double with SSE constraints */
    /* Tests floating point reloads */
    asm volatile (
        /* Double precision operation */
        "movsd %1, %%xmm0\n\t"
        "addsd %2, %%xmm0\n\t"
        "movsd %%xmm0, %0"
        : "=x" (out_double)       /* Output in SSE register */
        : "x" (d),                /* Input in SSE register */
          "xm" (g_double)         /* Input in SSE reg or memory */
        : "xmm0", "cc"
    );
    /* Convert double to int for accumulator */
    temp = (int)out_double;
    accumulator += temp;
    
    /* ASM 6: Multiple outputs with tied operands */
    /* Creates complex reload scenario */
    int in1 = a + 1;
    int in2 = a + 2;
    asm volatile (
        /* Multiple operations */
        "imull %2, %1\n\t"
        "addl %1, %0"
        : "+&r" (out_int1),       /* Read-write, early clobber */
          "=&r" (out_int2)        /* Early clobber */
        : "rm" (in1),             /* Input in reg/memory */
          "1" (in2)               /* Tied to output 1 */
        : "cc"
    );
    accumulator += out_int1 + out_int2;
    
    /* ASM 7: Vector-style operation (simulated) */
    /* Tests potential vector reloads */
    asm volatile (
        /* Simulate vector operation */
        "movl %1, %0\n\t"
        "shll $2, %0"
        : "=r" (temp)
        : "r" (a)
        : "cc"
    );
    accumulator += temp;
    
    /* ASM 8: Complex addressing with displacement */
    /* Forces address computation reloads */
    asm volatile (
        /* Load with complex address */
        "movl 4(%1,%2,4), %0"
        : "=r" (temp)
        : "r" (ptr),              /* Base pointer */
          "r" (a & 3)             /* Scaled index */
        : "memory"
    );
    accumulator += temp;
    
    return accumulator;
}

int main(int argc, char *argv[])
{
    /* Use argv to create variable inputs to prevent constant propagation */
    int base = (argc > 1) ? atoi(argv[1]) : 12345;
    
    /* Initialize variables with non-constant values */
    int int_val = base + g_int;
    long long llong_val = (long long)base * g_llong;
    float float_val = (float)base / 10.0f + g_float;
    double double_val = (double)base / 100.0 + g_double;
    int *ptr = &g_array[0];
    
    /* Call the function multiple times with different arguments */
    long long result1 = trigger_reloads(int_val, llong_val, float_val, double_val, ptr);
    long long result2 = trigger_reloads(int_val + 1, llong_val + 1, 
                                       float_val + 1.0f, double_val + 1.0, 
                                       &g_array[5]);
    
    /* Use results to prevent dead code elimination */
    printf("Result 1: %lld\n", result1);
    printf("Result 2: %lld\n", result2);
    printf("Total: %lld\n", result1 + result2);
    
    return (int)((result1 + result2) % 1000);
}
