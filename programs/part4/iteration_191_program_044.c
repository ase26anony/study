/* reload_coverage.c
 * Designed to trigger push_reload with secondary reload initialization
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
volatile int global_int = 12345;
volatile long long global_ll = 9876543210LL;
volatile float global_float = 3.14159f;
volatile double global_double = 2.718281828459045;

/* Function with multiple inline asm statements to trigger reloads */
static long long trigger_reloads(int a, long long b, float c, double d) {
    int out1, out2;
    long long out3;
    float out4;
    double out5;
    long long accumulator = 0;
    
    /* ASM 1: Mixed modes with specific register constraints */
    asm volatile (
        /* Template uses different size modifiers for different modes */
        "movl %1, %0\n\t"
        "addl $42, %0"
        : "=r" (out1)          /* Output in general reg, might need reload */
        : "rm" (a)             /* Input in reg/memory - forces choice */
        : "cc"                 /* Clobbers flags */
    );
    accumulator += out1;
    
    /* ASM 2: Requires secondary reload for constant address */
    asm volatile (
        "movq %1, %0\n\t"
        "xorq $0xFF, %0"
        : "=r" (out3)          /* Output constraint */
        : "ri" (b)             /* reg/imm - immediate might need secondary reload */
        : "cc"
    );
    accumulator += out3;
    
    /* ASM 3: Floating point with integer conversion - mixed mode reload */
    asm volatile (
        "cvttss2si %1, %0\n\t"  /* Convert float to int */
        : "=r" (out2)           /* Integer output */
        : "xm" (c)              /* SSE reg or memory - specific constraint */
        : /* no clobber */
    );
    accumulator += out2;
    
    /* ASM 4: Double with specific constraint that may need secondary reload */
    asm volatile (
        "movsd %1, %0\n\t"
        "addsd %2, %0"
        : "=x" (out5)           /* Must be in SSE register */
        : "xm" (d),             /* SSE reg or memory */
          "xm" (global_double)  /* Another SSE/memory operand */
        : /* no clobber */
    );
    /* Use the result to prevent elimination */
    accumulator += (long long)out5;
    
    /* ASM 5: Complex addressing mode that might need secondary reload */
    asm volatile (
        "leaq (%1,%2,1), %0\n\t"
        : "=r" (out3)
        : "r" (&global_int),    /* Address might need secondary reload */
          "r" (out1)            /* From previous asm */
        : "cc"
    );
    accumulator += out3;
    
    /* ASM 6: Multiple outputs with different constraints */
    asm volatile (
        "imull %2, %1\n\t"
        "movl %1, %0"
        : "=rm" (out1),         /* Output can be reg or memory */
          "=r" (out2)           /* Must be in register */
        : "r" (global_int),     /* Must be in register */
          "0" (out1),           /* Input/output operand */
          "1" (a)               /* Input/output operand */
        : "cc"
    );
    accumulator += out1 + out2;
    
    return accumulator;
}

/* Main function that provides varying inputs */
int main(int argc, char *argv[]) {
    int int_val;
    long long ll_val;
    float float_val;
    double double_val;
    
    /* Use argv to create varying inputs to prevent constant propagation */
    if (argc > 1) {
        int_val = atoi(argv[1]);
        ll_val = atoll(argv[1]) * 1000LL;
        float_val = (float)atof(argv[1]);
        double_val = atof(argv[1]) * 2.0;
    } else {
        int_val = 1000;
        ll_val = 5000000000LL;
        float_val = 1.2345f;
        double_val = 9.8765;
    }
    
    /* Mix with global volatiles */
    int_val += global_int;
    ll_val ^= global_ll;
    float_val *= global_float;
    double_val += global_double;
    
    /* Call the function that triggers reloads */
    long long result = trigger_reloads(int_val, ll_val, float_val, double_val);
    
    printf("Result: %lld\n", result);
    return 0;
}
