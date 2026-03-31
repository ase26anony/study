/* reload_coverage.c
 * Designed to trigger GCC's reload pass and cover push_reload initialization
 * lines 1381-1399 in reload.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
volatile int g_int = 42;
volatile long long g_llong = 0x123456789ABCDEFLL;
volatile float g_float = 3.14159f;
volatile double g_double = 2.718281828459045;

/* Vector types for additional reload complexity */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function with multiple inline asm statements to trigger reloads */
static long long trigger_reloads(int a, long long b, float c, double d, 
                                 v4si vec_int, v4sf vec_float) {
    long long result = 0;
    int out1, out2;
    long long out3;
    float out4;
    double out5;
    v4si out_vec_int;
    v4sf out_vec_float;
    
    /* ASM 1: Mixed integer types with specific register constraints */
    /* This forces reloads due to register class mismatches */
    asm volatile (
        /* Move with potential need for secondary reload */
        "movl %[in1], %[out1]\n\t"
        "addl %[in2], %[out1]\n\t"
        : [out1] "=r" (out1)          /* Output in general reg */
        : [in1] "rm" (a),             /* Input in reg/memory - may need reload */
          [in2] "i" (0x7FFFFFFF)      /* Large immediate - may need secondary reload */
        : "cc"                        /* Clobber condition codes */
    );
    result += out1;
    
    /* ASM 2: Floating point with integer conversion - different modes */
    /* Forces different inmode/outmode initialization */
    asm volatile (
        /* Convert float to int - requires mode change */
        "cvttss2si %[in_float], %[out_int]\n\t"
        : [out_int] "=r" (out2)       /* Integer output */
        : [in_float] "xm" (c)         /* Float input in SSE reg/memory */
        : /* No clobbers for this simple conversion */
    );
    result += out2;
    
    /* ASM 3: 64-bit operation with memory addressing */
    /* May require secondary reload for address computation */
    asm volatile (
        /* Load and add 64-bit values */
        "movq %[in_llong], %[out_llong]\n\t"
        "addq $0x1000, %[out_llong]\n\t"
        : [out_llong] "=&r" (out3)    /* Early clobber output */
        : [in_llong] "mr" (b)         /* Memory/register with possible addressing reload */
        : "cc"
    );
    result += out3;
    
    /* ASM 4: Double precision with specific constraint */
    /* Tests floating point reload paths */
    asm volatile (
        /* Double arithmetic */
        "addsd %[in_double], %[out_double], %[out_double]\n\t"
        : [out_double] "=x" (out5)    /* Output in SSE register */
        : [in_double] "xm" (d),       /* Input in SSE reg/memory */
          "0" (2.0)                   /* Input tied to output */
        : /* No clobbers */
    );
    result += (long long)out5;
    
    /* ASM 5: Vector operations - different register class */
    /* May require vector register reloads */
    asm volatile (
        /* Vector move and add */
        "movdqa %[in_vec], %[out_vec]\n\t"
        "paddd %[out_vec], %[out_vec], %[out_vec]\n\t"
        : [out_vec] "=x" (out_vec_int) /* Output in XMM register */
        : [in_vec] "xm" (vec_int)      /* Input in XMM reg/memory */
        : /* No clobbers */
    );
    
    /* Use vector result to affect final output */
    for (int i = 0; i < 4; i++) {
        result += out_vec_int[i];
    }
    
    /* ASM 6: Complex addressing mode with multiple constraints */
    /* High probability of requiring secondary reloads */
    asm volatile (
        /* Use global variable address - may need secondary reload */
        "movl g_int(%%rip), %[out]\n\t"
        : [out] "=r" (out1)           /* Output in register */
        : /* No inputs */
        : "memory"                    /* Clobber memory */
    );
    result += out1;
    
    /* ASM 7: In/out operand with '+' constraint */
    /* Tests combined input/output reload handling */
    {
        int combined = a;
        asm volatile (
            "addl $1, %[combined]\n\t"
            : [combined] "+rm" (combined)  /* Read-write operand */
            : /* No separate inputs */
            : "cc"
        );
        result += combined;
    }
    
    return result;
}

/* Main function that sets up test data and calls trigger function */
int main(int argc, char **argv) {
    /* Use argv to create variable inputs (prevents constant propagation) */
    int int_val = (argc > 1) ? atoi(argv[1]) : 100;
    long long llong_val = (argc > 2) ? atoll(argv[2]) : 1000LL;
    float float_val = (argc > 3) ? atof(argv[3]) : 1.5f;
    double double_val = (argc > 4) ? atof(argv[4]) : 2.5;
    
    /* Initialize vector values */
    v4si vec_int = {int_val, int_val + 1, int_val + 2, int_val + 3};
    v4sf vec_float = {float_val, float_val + 1.0f, float_val + 2.0f, float_val + 3.0f};
    
    /* Call function multiple times with different parameters */
    long long total = 0;
    for (int i = 0; i < 3; i++) {
        total += trigger_reloads(int_val + i, 
                                llong_val + i * 100LL,
                                float_val + i * 0.1f,
                                double_val + i * 0.2,
                                vec_int,
                                vec_float);
    }
    
    /* Use the result to prevent optimization */
    printf("Result: %lld\n", total);
    
    /* Additional test with global variables */
    total += trigger_reloads(g_int, g_llong, g_float, g_double, vec_int, vec_float);
    printf("Final result: %lld\n", total);
    
    return (total > 0) ? 0 : 1;
}
