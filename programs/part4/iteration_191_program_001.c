/* reload_test.c - Test program to trigger GCC's reload pass uncovered lines */
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
    
    /* ASM 1: Mixed types with specific register constraints */
    /* This should trigger reloads with secondary reload info */
    asm volatile (
        /* Move int to output with 'r' constraint, but complex addressing */
        "movl %[input1], %[output1]\n\t"
        /* Use the pointer in a memory operand that might need reload */
        "movl (%[ptr]), %[output2]\n\t"
        /* Try to force a secondary reload with constant pool address */
        "leaq g_array(%%rip), %%rax\n\t"
        "movl (%%rax), %[output2]"
        : [output1] "=r" (out1), 
          [output2] "=r" (out2)
        : [input1] "r" (a),
          [ptr] "r" (ptr),
          "m" (g_array)  /* Memory input that might need address reload */
        : "rax", "memory", "cc"
    );
    accumulator += out1 + out2;
    
    /* ASM 2: Floating point with integer conversion - different modes */
    /* This should set different inmode/outmode */
    asm volatile (
        /* Convert float to int - requires mode change */
        "cvttss2si %[flt_in], %[int_out]\n\t"
        /* Use immediate that might not fit in constraint */
        "addl $0x12345678, %[int_out]"
        : [int_out] "=r" (out1)
        : [flt_in] "x" (c),
          "i" (0x12345678)  /* Large immediate might need reload */
        : "cc"
    );
    accumulator += out1;
    
    /* ASM 3: 64-bit operations with specific constraints */
    /* Force use of specific registers */
    asm volatile (
        /* Complex addressing with displacement */
        "movq %[llong_in], %%rax\n\t"
        "addq $0x7FFFFFFFFFFFFFFF, %%rax\n\t"
        "movq %%rax, %[llong_out]"
        : [llong_out] "=r" (out3)
        : [llong_in] "r" (b),
          "i" (0x7FFFFFFFFFFFFFFF)  /* Large 64-bit immediate */
        : "rax", "cc"
    );
    accumulator += out3;
    
    /* ASM 4: Mixed float/double with memory constraints */
    /* This creates pressure on floating point registers */
    asm volatile (
        /* Load from memory, operate, store to output */
        "movsd %[dbl_in], %%xmm0\n\t"
        "addsd g_double(%%rip), %%xmm0\n\t"
        "movsd %%xmm0, %[dbl_out]\n\t"
        /* Also use float */
        "movss %[flt_in], %%xmm1\n\t"
        "mulss g_float(%%rip), %%xmm1\n\t"
        "movss %%xmm1, %[flt_out]"
        : [dbl_out] "=m" (out5),
          [flt_out] "=m" (out4)
        : [dbl_in] "m" (d),
          [flt_in] "m" (c),
          "m" (g_double),
          "m" (g_float)
        : "xmm0", "xmm1", "memory", "cc"
    );
    accumulator += (long long)out4 + (long long)out5;
    
    /* ASM 5: Vector-style operation (even without SIMD) */
    /* Multiple outputs with earlyclobber */
    asm volatile (
        /* Simulate vector operation */
        "movq %[in1], %[out1]\n\t"
        "movq %[in2], %[out2]\n\t"
        "addq %[out1], %[out2]"
        : [out1] "=&r" (out3),  /* Earlyclobber */
          [out2] "=r" (accumulator)  /* Output overlaps with input */
        : [in1] "r" (b),
          [in2] "0" (accumulator)  /* Matching constraint */
        : "cc"
    );
    
    return accumulator;
}

/* Wrapper to add more register pressure */
static long long reload_wrapper(int x)
{
    long long result = 0;
    int local_int = x * 2;
    long long local_llong = x * 3LL;
    float local_float = x * 1.5f;
    double local_double = x * 2.5;
    int local_array[5] = {x, x+1, x+2, x+3, x+4};
    
    /* Call multiple times with different args */
    for (int i = 0; i < 3; i++) {
        result += trigger_reloads(local_int + i, 
                                 local_llong + i,
                                 local_float + i,
                                 local_double + i,
                                 local_array);
    }
    
    return result;
}

int main(int argc, char **argv)
{
    long long total = 0;
    
    /* Use argv to create variant inputs */
    int base = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Create multiple call paths */
    for (int j = 0; j < 2; j++) {
        total += reload_wrapper(base + j * 100);
    }
    
    /* Use global vars to prevent optimization */
    total += g_int + g_llong + (long long)g_float + (long long)g_double;
    
    printf("Result: %lld\n", total);
    
    /* Also use inline asm in main for good measure */
    asm volatile (
        "movl %[val], %%eax\n\t"
        "addl $1, %%eax"
        : 
        : [val] "r" (g_int)
        : "eax", "cc"
    );
    
    return (int)(total % 1000);
}
