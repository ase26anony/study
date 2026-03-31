/* reload_coverage.c
 * Designed to trigger GCC's reload pass push_reload function
 * with secondary reload initialization.
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
volatile int global_int = 12345;
volatile long long global_ll = 9876543210LL;
volatile float global_float = 3.14159f;
volatile double global_double = 2.718281828459045;

/* Function with complex inline assembly to trigger reloads */
static long long trigger_reloads(int a, long long b, float c, double d) {
    int out1, out2;
    long long out3;
    float out4;
    double out5;
    long long accumulator = 0;
    
    /* ASM 1: Mixed types with specific register constraints
     * Forces reloads due to mismatched constraints */
    asm volatile (
        /* Move int to output with 'r' constraint, but input is memory */
        "movl %1, %0\n\t"
        : "=r" (out1)          /* Output in general register */
        : "m" (global_int)     /* Input from memory */
        : /* no clobbers */
    );
    accumulator += out1;
    
    /* ASM 2: Requires secondary reload - constant address loading
     * This often needs secondary reload on many architectures */
    asm volatile (
        "leaq %1, %0\n\t"      /* Load address - may need secondary reload */
        : "=r" (out3)          /* Output in register */
        : "i" (&global_ll)     /* Input is constant address */
        : /* no clobbers */
    );
    accumulator += out3;
    
    /* ASM 3: Floating point with integer constraints
     * Creates mode mismatches */
    asm volatile (
        "movd %1, %0\n\t"      /* Move float through integer register */
        : "=r" (out2)          /* Output in integer register */
        : "x" (c)              /* Input in SSE register */
        : /* no clobbers */
    );
    accumulator += out2;
    
    /* ASM 4: Multiple operands with different modes
     * Complex constraints that force reload decisions */
    asm volatile (
        "imulq %2, %1\n\t"
        "addq %1, %0\n\t"
        : "+r" (accumulator), "=r" (out3)
        : "rm" (b), "0" (accumulator), "1" (b)
        : "cc"
    );
    
    /* ASM 5: Specific register constraints that conflict
     * Forces register shuffling */
    asm volatile (
        "mov %1, %%eax\n\t"
        "add %2, %%eax\n\t"
        "mov %%eax, %0\n\t"
        : "=r" (out1)
        : "r" (a), "i" (1000)
        : "eax", "cc"
    );
    accumulator += out1;
    
    /* ASM 6: Vector-style operation (if supported)
     * Using larger data types */
    {
        long long vec_in[2] = {b, b + 1};
        long long vec_out[2];
        
        asm volatile (
            "movq %1, %0\n\t"
            "movq %2, %3\n\t"
            : "=r" (vec_out[0]), "=r" (vec_out[1])
            : "rm" (vec_in[0]), "rm" (vec_in[1])
            : /* no clobbers */
        );
        accumulator += vec_out[0] + vec_out[1];
    }
    
    /* ASM 7: Memory constraint with offset
     * May require address computation reload */
    asm volatile (
        "movq 8(%1), %0\n\t"
        : "=r" (out3)
        : "r" (&global_ll - 1)  /* Complex address */
        : "memory"
    );
    accumulator += out3;
    
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
        int_val = 42;
        ll_val = 123456789LL;
        float_val = 1.2345f;
        double_val = 5.6789;
    }
    
    /* Call the function multiple times with different arguments */
    long long result1 = trigger_reloads(int_val, ll_val, float_val, double_val);
    long long result2 = trigger_reloads(int_val + 1, ll_val + 1, 
                                       float_val + 1.0f, double_val + 1.0);
    
    /* Use results to prevent dead code elimination */
    printf("Result 1: %lld\n", result1);
    printf("Result 2: %lld\n", result2);
    printf("Final: %lld\n", result1 + result2);
    
    return (int)((result1 + result2) % 1000);
}
