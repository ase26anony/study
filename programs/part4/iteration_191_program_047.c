/* reload_coverage.c
 * Designed to trigger push_reload with secondary reload initialization
 * Compile with: gcc -O1 -fno-inline -fdump-rtl-reload -S reload_coverage.c
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
        /* Move int to output with 'r' constraint, but input might need secondary reload */
        "movl %1, %0\n\t"
        : "=r" (out1)          /* Output in general register */
        : "irm" (a)            /* Input: immediate, register, or memory - may need secondary */
        : /* no clobbers */
    );
    accumulator += out1;
    
    /* ASM 2: Long long with memory addressing that may require secondary reloads */
    asm volatile (
        /* Complex addressing that might need reload */
        "movq %1, %0\n\t"
        : "=r" (out3)          /* Output in register */
        : "m" (global_ll)      /* Memory operand that might need address reload */
        : "memory"
    );
    accumulator += out3;
    
    /* ASM 3: Floating point with general register constraints
     * May require secondary reloads on some architectures */
    asm volatile (
        /* Move float through integer register (requires secondary reload) */
        "movd %1, %0\n\t"
        : "=r" (out2)          /* Integer register output */
        : "x" (c)              /* SSE/float register input */
        : /* no clobbers */
    );
    accumulator += out2;
    
    /* ASM 4: Double with specific constraints and clobbers
     * Creates pressure on register allocator */
    asm volatile (
        "movsd %1, %0\n\t"
        : "=x" (out5)          /* SSE register output */
        : "xm" (d)             /* SSE register or memory */
        : /* no clobbers */
    );
    /* Use the result */
    accumulator += (long long)out5;
    
    /* ASM 5: Multiple operands with conflicting requirements */
    asm volatile (
        "imull %1, %0\n\t"     /* Multiply and store in output */
        : "+r" (out1)          /* Read-write operand */
        : "irm" (global_int)   /* Complex constraint */
        : "cc"                 /* Condition codes clobbered */
    );
    accumulator += out1;
    
    /* ASM 6: Inline asm with immediate that might not fit
     * Could trigger secondary reload for constant */
    asm volatile (
        "addl $0x12345678, %0\n\t"  /* Large immediate */
        : "+r" (out1)
        : /* no inputs */
        : "cc"
    );
    accumulator += out1;
    
    /* ASM 7: Vector-style operation (if supported)
     * Using 64-bit mode to potentially trigger different reload paths */
    {
        long long vec_in = b;
        long long vec_out;
        asm volatile (
            "movq %1, %0\n\t"
            "notq %0\n\t"      /* Simple operation */
            : "=r" (vec_out)
            : "r" (vec_in)
            : /* no clobbers */
        );
        accumulator += vec_out;
    }
    
    return accumulator;
}

/* Helper to ensure values aren't compile-time constants */
static int get_seed(int argc, char **argv) {
    if (argc > 1) return atoi(argv[1]);
    return 42;
}

int main(int argc, char **argv) {
    int seed = get_seed(argc, argv);
    
    /* Create non-constant inputs */
    int a = seed * 3;
    long long b = seed * 5LL;
    float c = seed * 0.7f;
    double d = seed * 1.3;
    
    /* Mix in global values */
    a += global_int;
    b += global_ll;
    c += global_float;
    d += global_double;
    
    /* Trigger the reload-intensive function */
    long long result = trigger_reloads(a, b, c, d);
    
    /* Use the result to prevent optimization */
    printf("Result: %lld\n", result);
    
    return (result > 0) ? 0 : 1;
}
