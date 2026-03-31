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
    int *out6;
    
    /* ASM 1: Mixed types with register constraints likely requiring reloads */
    /* This should trigger secondary reloads for constant addresses */
    asm volatile (
        /* Move with different sized operands */
        "movl %[input1], %[output1]\n\t"
        "movq %[input2], %[output3]\n\t"
        /* Use memory operand with complex addressing */
        "movl (%[input5], %[input1], 4), %[output2]"
        : [output1] "=r" (out1),     /* General purpose reg constraint */
          [output2] "=r" (out2),     /* Another GP reg */
          [output3] "=r" (out3)      /* 64-bit register */
        : [input1] "r" (a),          /* Input in register */
          [input2] "r" (b),          /* 64-bit input */
          [input5] "r" (ptr)         /* Pointer input */
        : "memory"                   /* Clobber memory */
    );
    accumulator += out1 + out2 + out3;
    
    /* ASM 2: Floating point with constraints that may need secondary reloads */
    /* Using specific x87/SSE constraints for x86 */
    asm volatile (
        "movss %[fin], %[fout]\n\t"
        "cvtsd2ss %[din], %[dout]"
        : [fout] "=x" (out4),        /* SSE/XMM register constraint */
          [dout] "=x" (out5)         /* Another XMM register */
        : [fin] "x" (c),             /* Float input in XMM */
          [din] "x" (d)              /* Double input in XMM */
        : /* No clobbers for XMM regs in this simple template */
    );
    accumulator += (long long)(out4 * 1000) + (long long)(out5 * 1000);
    
    /* ASM 3: Multiple constraints and clobbers to increase reload pressure */
    /* Using specific register constraints and immediate operands */
    asm volatile (
        "leaq (%[base], %[index], 4), %[result]\n\t"
        "addl $0x12345678, %[result32]"
        : [result] "=&r" (out6),     /* Early clobber reg constraint */
          [result32] "=r" (out1)     /* Another output */
        : [base] "r" (ptr),          /* Base register */
          [index] "r" (a & 3),       /* Index register */
          "i" (0x12345678)           /* Immediate operand */
        : "cc"                       /* Clobber condition codes */
    );
    accumulator += (long long)((char *)out6 - (char *)ptr);
    
    /* ASM 4: Vector-style operation with mismatched modes */
    /* Using 64-bit operations on 32-bit values */
    asm volatile (
        "movq %[in1], %%mm0\n\t"     /* MMX register */
        "paddd %[in2], %%mm0\n\t"
        "movq %%mm0, %[out]"
        : [out] "=r" (out3)          /* 64-bit output */
        : [in1] "r" ((long long)(a * 0x100000001LL)), /* 64-bit immediate */
          [in2] "r" ((long long)(g_int * 0x100000001LL)) /* Another 64-bit */
        : "%mm0"                     /* Clobber MMX register */
    );
    accumulator += out3;
    
    /* ASM 5: Complex addressing mode that might need secondary reload */
    /* Using displacement that might not fit in instruction */
    asm volatile (
        "movl 0x%a[offset](%[base]), %[out]"  /* Large offset */
        : [out] "=r" (out1)
        : [base] "r" (ptr),
          [offset] "i" (256)         /* Immediate offset */
        : "memory"
    );
    accumulator += out1;
    
    return accumulator;
}

int main(int argc, char **argv)
{
    long long result;
    int seed;
    
    /* Use argv for variability to prevent constant propagation */
    seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Initialize with somewhat random values */
    int int_val = rand() % 1000;
    long long llong_val = (long long)rand() * rand();
    float float_val = (float)rand() / RAND_MAX * 100.0f;
    double double_val = (double)rand() / RAND_MAX * 100.0;
    
    /* Mix with global volatiles */
    int_val += g_int;
    llong_val += g_llong;
    float_val += g_float;
    double_val += g_double;
    
    /* Call the function with complex inline assembly */
    result = trigger_reloads(int_val, llong_val, float_val, double_val, g_array);
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %lld\n", result);
    
    return (result > 0) ? 0 : 1;
}
