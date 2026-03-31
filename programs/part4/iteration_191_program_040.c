/* reload_test.c - Test program to trigger push_reload uncovered lines */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
volatile int g_int = 42;
volatile long long g_llong = 9876543210LL;
volatile float g_float = 3.14159f;
volatile double g_double = 2.718281828459045;
volatile void *g_ptr = (void*)0x12345678;

/* Function with complex inline assembly to trigger reloads */
static long long trigger_reloads(int a, long long b, float c, double d, void *ptr)
{
    long long accumulator = 0;
    int out1, out2;
    long long out3;
    float out4;
    double out5;
    void *out6;
    
    /* 
     * Assembly block 1: Mixed integer types with specific register constraints
     * This forces reloads due to register class mismatches
     */
    asm volatile (
        /* Move int to output with specific register constraint */
        "movl %1, %0\n\t"
        /* Use the value in a computation */
        "addl $100, %0"
        : "=r" (out1)          /* Output in general register */
        : "rm" (a)             /* Input in register or memory */
        : "cc"                 /* Clobbers condition codes */
    );
    accumulator += out1;
    
    /*
     * Assembly block 2: 64-bit operations with memory constraints
     * May require secondary reloads for address computation
     */
    asm volatile (
        /* Load 64-bit value and modify */
        "movq %1, %0\n\t"
        "xorq $0xFF, %0"
        : "=r" (out3)          /* Output must be in register */
        : "m" (b)              /* Input from memory location */
        : "cc"
    );
    accumulator += out3;
    
    /*
     * Assembly block 3: Floating point with integer conversion
     * Forces mode changes and potential secondary reloads
     */
    asm volatile (
        /* Convert float to int in register */
        "cvttss2si %1, %0\n\t"
        "subl $50, %0"
        : "=r" (out2)          /* Integer output */
        : "x" (c)              /* SSE register input for float */
        : "cc"
    );
    accumulator += out2;
    
    /*
     * Assembly block 4: Double precision with specific constraints
     * Uses different register classes
     */
    asm volatile (
        /* Double precision operation */
        "movsd %1, %0\n\t"
        "mulsd %2, %0"
        : "=x" (out5)          /* Output in SSE register */
        : "xm" (d),            /* Input in SSE reg or memory */
          "xm" (g_double)      /* Global variable input */
        : "cc"
    );
    /* Convert double to long long for accumulator */
    accumulator += (long long)out5;
    
    /*
     * Assembly block 5: Pointer manipulation
     * May require secondary reloads for address computation
     */
    asm volatile (
        /* Pointer arithmetic */
        "movq %1, %0\n\t"
        "addq $16, %0"
        : "=r" (out6)          /* Pointer output */
        : "rm" (ptr)           /* Input pointer */
        : "cc"
    );
    accumulator += (long long)out6;
    
    /*
     * Assembly block 6: Complex constraints forcing spill/reload
     * Multiple outputs with early clobber
     */
    asm volatile (
        /* Two-output operation */
        "leaq (%1,%2), %0\n\t"
        "movl %3, %k4"
        : "=&r" (out3),        /* Early clobber output */
          "=r" (out1)          /* Second output */
        : "r" (a),             /* Input in register */
          "rm" (g_int),        /* Global variable */
          "0" (out3)           /* Tied operand */
        : "cc"
    );
    accumulator += out3 + out1;
    
    /*
     * Assembly block 7: Vector-style operation (simulated)
     * Multiple constraints that may require secondary reloads
     */
    {
        long long tmp_in = b + 1000;
        asm volatile (
            /* Simulated vector operation */
            "movq %1, %0\n\t"
            "rorq $8, %0"
            : "=r" (out3)
            : "ri" (tmp_in)    /* Register or immediate */
            : "cc"
        );
        accumulator += out3;
    }
    
    return accumulator;
}

/* Main function that sets up test data */
int main(int argc, char **argv)
{
    /* Use argv to create variant inputs preventing constant propagation */
    int base = argc > 1 ? atoi(argv[1]) : 100;
    
    /* Initialize test variables with non-constant values */
    int int_val = base + g_int;
    long long llong_val = (long long)base * 100 + g_llong;
    float float_val = (float)base / 10.0f + g_float;
    double double_val = (double)base / 3.0 + g_double;
    void *ptr_val = (void*)((long long)g_ptr + base);
    
    /* Call the function that triggers reloads */
    long long result = trigger_reloads(int_val, llong_val, float_val, 
                                      double_val, ptr_val);
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %lld\n", result);
    
    /* Additional test with different inputs */
    result += trigger_reloads(int_val * 2, llong_val / 2, 
                             float_val * 2.0f, double_val * 0.5, 
                             (char*)ptr_val + 100);
    
    printf("Final result: %lld\n", result);
    
    return (result > 0) ? 0 : 1;
}
