/* reload_test.c - Test program to trigger push_reload uncovered lines */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
volatile int g_int = 42;
volatile long long g_llong = 123456789012345LL;
volatile float g_float = 3.14159f;
volatile double g_double = 2.718281828459045;
volatile int g_array[10] = {0,1,2,3,4,5,6,7,8,9};

/* Function with complex inline assembly to trigger reloads */
static long long trigger_reloads(int a, long long b, float c, double d, int *ptr)
{
    long long accumulator = 0;
    int out1, out2;
    long long out3;
    float out4;
    double out5;
    int *out_ptr;
    
    /* ASM 1: Mixed types with specific register constraints */
    /* This should trigger secondary reloads for constant addresses */
    asm volatile (
        /* Template using different size modifiers */
        "movl %[in1], %[out1]\n\t"
        "movq %[in2], %[out3]\n\t"
        "movss %[in3], %[out4]\n\t"
        : [out1] "=r" (out1),      /* General purpose reg constraint */
          [out3] "=r" (out3),      /* 64-bit reg constraint */
          [out4] "=x" (out4)       /* SSE/vector reg constraint */
        : [in1] "rm" (a),          /* Register or memory - may need reload */
          [in2] "rm" (b),          /* 64-bit value may need reload */
          [in3] "xm" (c)           /* SSE reg or memory */
        : "memory", "cc"
    );
    accumulator += out1 + out3 + (long long)out4;
    
    /* ASM 2: Memory operand with complex addressing requiring secondary reload */
    /* Using 'm' constraint with indexed addressing */
    asm volatile (
        "leaq (%[base], %[index], 4), %[out_ptr]\n\t"
        "movl (%[out_ptr]), %[out2]\n\t"
        : [out_ptr] "=r" (out_ptr),  /* Output pointer - may need secondary reload */
          [out2] "=r" (out2)
        : [base] "r" (ptr),          /* Base pointer */
          [index] "r" (a & 7)        /* Scaled index */
        : "memory"
    );
    accumulator += out2 + (long long)(out_ptr - ptr);
    
    /* ASM 3: Floating point with specific constraints */
    /* Force use of specific register classes */
    asm volatile (
        "addsd %[in4], %[in4], %[out5]\n\t"
        "cvtsd2ss %[out5], %[out4]\n\t"
        : [out5] "=x" (out5),       /* SSE2 double reg */
          [out4] "=x" (out4)        /* SSE float reg */
        : [in4] "x" (d)             /* Input in SSE reg */
        : "cc"
    );
    accumulator += (long long)out5 + (long long)out4;
    
    /* ASM 4: Multiple outputs with conflicting constraints */
    /* This often triggers complex reload scenarios */
    asm volatile (
        "imulq %[in2], %[in2]\n\t"
        "movq %[in2], %[out3]\n\t"
        "movl %[in1], %[out1]\n\t"
        : [out3] "=&r" (out3),     /* Early clobber - can't share input reg */
          [out1] "=r" (out1)
        : [in2] "0" (b),           /* Same as output 0 - creates conflict */
          [in1] "rm" (g_int)       /* Global may need reload from memory */
        : "cc"
    );
    accumulator += out3 + out1;
    
    /* ASM 5: Vector-style operation with specific constraints */
    /* Using immediate constraints that may need secondary reloads */
    int imm = 255;
    asm volatile (
        "andl %[imm], %[out1]\n\t"
        "shrl $2, %[out1]\n\t"
        : [out1] "+r" (out1)
        : [imm] "i" (imm)          /* Immediate constraint */
        : "cc"
    );
    accumulator += out1;
    
    return accumulator;
}

/* Secondary function with different reload patterns */
static int more_reloads(int x, int y)
{
    int result;
    long long temp;
    
    /* ASM with 'm' constraint and complex address computation */
    asm volatile (
        "movl (%[addr], %[idx], 4), %[res]\n\t"
        "addl %[y], %[res]\n\t"
        : [res] "=r" (result)
        : [addr] "r" (g_array),    /* Global array base */
          [idx] "r" (x & 3),       /* Index */
          [y] "rm" (y)             /* May need reload */
        : "memory"
    );
    
    /* ASM with output in specific register class */
    asm volatile (
        "movq %%rax, %[temp]\n\t"
        "addq $1, %[temp]\n\t"
        : [temp] "=r" (temp)
        : 
        : "rax", "cc"
    );
    
    return result + (int)temp;
}

int main(int argc, char *argv[])
{
    /* Use argv to create variable inputs preventing constant propagation */
    int arg_int = argc > 1 ? atoi(argv[1]) : 100;
    long long arg_llong = argc > 2 ? atoll(argv[2]) : 999999999LL;
    float arg_float = argc > 3 ? atof(argv[3]) : 1.2345f;
    double arg_double = argc > 4 ? atof(argv[4]) : 5.6789;
    
    /* Call reload-intensive functions */
    long long result1 = trigger_reloads(arg_int, arg_llong, arg_float, 
                                       arg_double, g_array);
    int result2 = more_reloads(arg_int, g_int);
    
    /* Use results to prevent dead code elimination */
    printf("Result1: %lld\n", result1);
    printf("Result2: %d\n", result2);
    printf("Combined: %lld\n", result1 + result2);
    
    return (int)(result1 + result2) & 0xFF;
}
