/* reload_test.c - Test program to trigger reload.cc push_reload uncovered lines */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
volatile int g_int = 12345;
volatile long long g_llong = 9876543210LL;
volatile float g_float = 3.14159f;
volatile double g_double = 2.718281828459045;
volatile int g_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

/* Function with complex inline assembly to trigger reloads */
static long long trigger_reloads(int a, long long b, float c, double d, int *ptr)
{
    long long accumulator = 0;
    int out1, out2;
    long long out3;
    float out4;
    double out5;
    
    /* ASM 1: Mixed integer types with specific register constraints */
    /* This may require secondary reloads for constant addresses */
    asm volatile (
        "movl %[input1], %%eax\n\t"          /* Force use of eax */
        "addl %[input2], %%eax\n\t"
        "movl %%eax, %[output]\n\t"
        : [output] "=r" (out1)               /* Output constraint */
        : [input1] "rm" (a),                 /* Input can be reg or mem */
          [input2] "i" (g_int)               /* Immediate may need reload */
        : "eax", "cc"                        /* Clobber eax and flags */
    );
    accumulator += out1;
    
    /* ASM 2: 64-bit operations with memory constraints */
    /* May trigger secondary reloads for 64-bit constants */
    asm volatile (
        "movq %[in1], %[out]\n\t"
        "addq %[in2], %[out]\n\t"
        : [out] "=&r" (out3)                 /* Early clobber output */
        : [in1] "r" (b),                     /* Input in register */
          [in2] "rm" (g_llong)               /* May be in memory */
        : "cc"
    );
    accumulator += out3;
    
    /* ASM 3: Floating point with integer conversion */
    /* Different modes (SFmode vs SImode) */
    asm volatile (
        "cvttss2si %[flt_in], %%eax\n\t"     /* Convert float to int */
        "movl %%eax, %[int_out]\n\t"
        : [int_out] "=r" (out2)
        : [flt_in] "x" (c)                   /* SSE register constraint */
        : "eax", "cc"
    );
    accumulator += out2;
    
    /* ASM 4: Double precision with memory addressing */
    /* Complex addressing may require reloads */
    asm volatile (
        "movsd %[dbl_in], %%xmm0\n\t"
        "addsd %[mem_in], %%xmm0\n\t"
        "movsd %%xmm0, %[dbl_out]\n\t"
        : [dbl_out] "=x" (out5)              /* SSE register output */
        : [dbl_in] "x" (d),                  /* SSE register input */
          [mem_in] "m" (g_double)            /* Memory input */
        : "xmm0", "cc"
    );
    /* Use the double output indirectly */
    accumulator += (long long)out5;
    
    /* ASM 5: Complex constraints with multiple outputs */
    /* May trigger push_reload with secondary reload info */
    {
        int temp1, temp2;
        asm volatile (
            "imull %[val1], %[val2]\n\t"
            "movl %[val2], %[res1]\n\t"
            "leal (%[ptr],%[val2],4), %[res2]\n\t"
            : [res1] "=r" (temp1),
              [res2] "=r" (temp2)
            : [val1] "r" (a),
              [val2] "0" (out1),             /* Same as output 0 */
              [ptr] "r" (ptr)                /* Pointer in register */
            : "cc"
        );
        accumulator += temp1 + temp2;
    }
    
    /* ASM 6: Vector-style operation (simulated) */
    /* Different register class constraints */
    {
        long long vec_op;
        asm volatile (
            "movq %[in1], %%rax\n\t"
            "addq %[in2], %%rax\n\t"
            "movq %%rax, %[out]\n\t"
            : [out] "=r" (vec_op)
            : [in1] "r" (accumulator),
              [in2] "i" (0x1000)             /* Immediate may need reload */
            : "rax", "cc"
        );
        accumulator = vec_op;
    }
    
    return accumulator;
}

/* Wrapper function to add more register pressure */
static long long wrapper_function(int x)
{
    /* Create more complex calling context */
    int local_int = x * 2;
    long long local_llong = x * 3LL;
    float local_float = x * 1.5f;
    double local_double = x * 2.5;
    
    /* Use global array element address */
    int *ptr = &g_array[x % 10];
    
    return trigger_reloads(local_int, local_llong, local_float, 
                          local_double, ptr) + x;
}

int main(int argc, char *argv[])
{
    long long total = 0;
    int i, iterations;
    
    /* Use command line argument or default */
    iterations = (argc > 1) ? atoi(argv[1]) : 5;
    if (iterations <= 0) iterations = 5;
    
    printf("Testing reload pass with %d iterations...\n", iterations);
    
    for (i = 0; i < iterations; i++) {
        /* Mix up the inputs to prevent constant propagation */
        int base = i * 7 + argc;
        total += wrapper_function(base);
        
        /* Modify globals to prevent optimization */
        g_int += i;
        g_float += 0.1f * i;
    }
    
    printf("Result: %lld\n", total);
    
    /* Use the result to prevent dead code elimination */
    if (total > 1000000) {
        printf("Large result detected\n");
    }
    
    return (int)(total % 1000);
}
