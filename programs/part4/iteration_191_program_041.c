/* reload_coverage.c - Test program to cover reload.cc push_reload initialization */
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
    int *out_ptr;
    
    /* ASM 1: Mixed modes with specific register constraints */
    /* This should trigger reloads with secondary reload info */
    asm volatile (
        /* Move int to output with specific constraint */
        "movl %1, %0\n\t"
        /* Use the value in a computation */
        "addl $100, %0"
        : "=r" (out1)          /* Output in general reg */
        : "irm" (a)            /* Input: immediate, reg, or memory */
        : "cc"                 /* Clobbers condition codes */
    );
    accumulator += out1;
    
    /* ASM 2: Memory operand with complex addressing requiring reload */
    /* Likely to need secondary reloads for address computation */
    asm volatile (
        "movq (%1), %0\n\t"    /* Load from memory address */
        "addq %2, %0"          /* Add immediate/register */
        : "=r" (out3)          /* Output in 64-bit register */
        : "r" (&g_array[g_int % 10]),  /* Complex address computation */
          "ri" (b)             /* Register or immediate */
        : "memory"             /* Clobbers memory */
    );
    accumulator += out3;
    
    /* ASM 3: Floating point with general register constraints */
    /* Forces reload between FP and GP registers */
    asm volatile (
        "movd %1, %0\n\t"      /* Move float to GP register */
        "paddd %0, %0\n\t"     /* Some SIMD operation */
        : "=x" (out1)          /* Output in SSE register */
        : "r" (g_int)          /* Input in general register */
        : "xmm0"               /* Clobber xmm0 */
    );
    accumulator += out1;
    
    /* ASM 4: Multiple outputs with different constraints */
    /* Tests initialization of multiple reload entries */
    asm volatile (
        "mov %2, %0\n\t"
        "mov %3, %1"
        : "=r" (out1), "=r" (out2)  /* Two outputs */
        : "r" (a), "i" (0x7FFFFFFF) /* Register and large immediate */
        : /* No clobbers */
    );
    accumulator += out1 + out2;
    
    /* ASM 5: Output with memory constraint */
    /* Tests out_reg initialization */
    asm volatile (
        "movq %1, %0"
        : "=m" (g_llong)       /* Output to memory */
        : "r" (accumulator)    /* Input from register */
        : "memory"
    );
    
    /* ASM 6: In/Out operand with '+' constraint */
    /* Tests both in_reg and out_reg initialization */
    asm volatile (
        "inc %0"
        : "+r" (out1)          /* Read-write operand */
        : /* No inputs */
        : "cc"
    );
    accumulator += out1;
    
    /* ASM 7: Vector-type operation with specific register class */
    /* May require secondary reloads on some architectures */
    {
        typedef int v4si __attribute__((vector_size(16)));
        v4si v1 = {a, a+1, a+2, a+3};
        v4si v2 = {4, 5, 6, 7};
        v4si v3;
        
        asm volatile (
            "paddd %1, %2, %0"
            : "=x" (v3)        /* Output in vector register */
            : "x" (v1), "x" (v2) /* Inputs in vector registers */
            : /* No clobbers */
        );
        accumulator += v3[0];
    }
    
    /* ASM 8: String instruction with implicit registers */
    /* Tests reloads with specific register requirements */
    {
        char src[16] = "test_string";
        char dst[16];
        
        asm volatile (
            "movsq"
            : /* No outputs */
            : "S" (src), "D" (dst), "c" (2LL) /* Source, Dest, Count */
            : "memory"
        );
    }
    
    /* ASM 9: Operand with alternative constraints */
    /* Tests reload selection logic */
    asm volatile (
        "add %1, %0"
        : "=r" (out1)
        : "r,i" (g_int)        /* Register or immediate */
        : "cc"
    );
    accumulator += out1;
    
    /* ASM 10: Large immediate that may require constant pool */
    /* Likely to need secondary reload for constant loading */
    asm volatile (
        "mov %1, %0"
        : "=r" (out3)
        : "i" (0x123456789ABCDEF0LL) /* Large 64-bit immediate */
        : /* No clobbers */
    );
    accumulator += out3;
    
    return accumulator;
}

int main(int argc, char *argv[])
{
    /* Use argv to create variable values to prevent constant propagation */
    int base = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Initialize test variables with non-constant values */
    int int_val = base + g_int;
    long long llong_val = (long long)base * g_llong;
    float float_val = (float)base * g_float;
    double double_val = (double)base * g_double;
    int *ptr_val = &g_array[base % 10];
    
    /* Call the function that triggers reloads */
    long long result = trigger_reloads(int_val, llong_val, float_val, 
                                      double_val, ptr_val);
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %lld\n", result);
    
    /* Additional computation using global variables */
    g_int += result;
    g_llong -= result;
    
    return (int)(result % 1000);
}
