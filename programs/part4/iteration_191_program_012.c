/* reload_coverage.c - Test program to cover push_reload initialization */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
volatile int g_int = 12345;
volatile long long g_llong = 9876543210LL;
volatile float g_float = 3.14159f;
volatile double g_double = 2.718281828459045;
volatile void *g_ptr = (void*)0x1000;

/* Function with complex inline assembly to trigger reloads */
static long long trigger_reloads(int a, long long b, float c, double d, void *ptr)
{
    long long accumulator = 0;
    int out_int1, out_int2;
    long long out_llong;
    float out_float;
    double out_double;
    void *out_ptr;
    
    /* Assembly 1: Mixed modes with specific register constraints */
    asm volatile (
        /* Move int to output with 'r' constraint - may need reload */
        "movl %1, %0\n\t"
        /* Use the value in a dummy operation to ensure it's used */
        "addl $1, %0"
        : "=r" (out_int1)          /* Output in general reg */
        : "r" (a)                  /* Input in general reg */
        : "cc"                     /* Clobber flags */
    );
    accumulator += out_int1;
    
    /* Assembly 2: Floating point with memory constraints */
    asm volatile (
        /* Load float, do operation, store result */
        "movss %1, %%xmm0\n\t"
        "mulss %2, %%xmm0\n\t"
        "movss %%xmm0, %0"
        : "=m" (out_float)         /* Memory output */
        : "m" (c),                 /* Memory input */
          "m" (g_float)            /* Global memory input */
        : "xmm0", "memory"         /* Clobber xmm0 and memory */
    );
    /* Use result to prevent elimination */
    accumulator += (long long)(out_float * 100);
    
    /* Assembly 3: 64-bit operations with specific constraints */
    asm volatile (
        /* Complex addressing mode that might need secondary reload */
        "movq %1, %%rax\n\t"
        "addq %2, %%rax\n\t"
        "movq %%rax, %0"
        : "=r" (out_llong)         /* Output in general reg */
        : "r" (b),                 /* Input in general reg */
          "i" (0x1000)             /* Immediate that might need reload */
        : "rax", "cc"              /* Clobber rax and flags */
    );
    accumulator += out_llong;
    
    /* Assembly 4: Pointer manipulation with offset */
    asm volatile (
        /* Pointer arithmetic that might need reload */
        "leaq 16(%1), %0"
        : "=r" (out_ptr)           /* Output in general reg */
        : "r" (ptr)                /* Input in general reg */
        : "cc"
    );
    accumulator += (long long)out_ptr;
    
    /* Assembly 5: Double precision with mismatched constraints */
    asm volatile (
        /* Move between regs with different constraints */
        "movsd %1, %%xmm1\n\t"
        "addsd %2, %%xmm1\n\t"
        "movsd %%xmm1, %0"
        : "=rm" (out_double)       /* Register or memory output */
        : "rf" (d),                /* Register or float reg input */
          "fm" (g_double)          /* Float reg or memory input */
        : "xmm1", "memory"
    );
    accumulator += (long long)out_double;
    
    /* Assembly 6: Integer with immediate that needs secondary reload */
    asm volatile (
        "imull %1, %0\n\t"
        "addl $0x12345678, %0"     /* Large immediate */
        : "+r" (out_int2)          /* Read-write operand */
        : "r" (g_int)              /* Global in register */
        : "cc"
    );
    accumulator += out_int2;
    
    return accumulator;
}

/* Main function with varied inputs */
int main(int argc, char **argv)
{
    long long result;
    int arg_int;
    long long arg_llong;
    float arg_float;
    double arg_double;
    
    /* Use argv to create non-constant inputs */
    arg_int = (argc > 1) ? atoi(argv[1]) : 1000;
    arg_llong = (argc > 2) ? atoll(argv[2]) : 5000000000LL;
    arg_float = (argc > 3) ? atof(argv[3]) : 1.2345f;
    arg_double = (argc > 4) ? atof(argv[4]) : 5.6789;
    
    /* Call function with all argument types */
    result = trigger_reloads(arg_int, arg_llong, arg_float, arg_double, g_ptr);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %lld\n", result);
    
    return (result > 0) ? 0 : 1;
}
