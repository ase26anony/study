/* reload_coverage.c - Test program to cover push_reload initialization */
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
    long long out3, out4;
    float out5;
    double out6;
    long long accumulator = 0;
    
    /* ASM 1: Mixed modes with specific register constraints */
    /* This should trigger reloads with secondary reload info */
    asm volatile (
        /* Move int to output with 'r' constraint - may need secondary reload */
        "movl %1, %0\n\t"
        /* Additional dummy operation to use more operands */
        "addl $0x1234, %0"
        : "=r" (out1)          /* Output in general reg */
        : "i" (0xABCD)         /* Immediate that might need reload */
        : "cc"                 /* Clobber flags */
    );
    accumulator += out1;
    
    /* ASM 2: Floating point with memory constraints */
    /* Forces reload between FP and general registers */
    asm volatile (
        /* Simulate a FP operation with memory operand */
        "movss %1, %%xmm0\n\t"
        "cvttss2si %%xmm0, %0"
        : "=r" (out2)          /* Output in general reg */
        : "m" (c)              /* Memory constraint on float */
        : "xmm0", "cc"
    );
    accumulator += out2;
    
    /* ASM 3: 64-bit operations with specific constraints */
    /* May require secondary reloads for 64-bit constants */
    asm volatile (
        /* 64-bit move with possible constant pool reload */
        "movq %1, %0\n\t"
        "addq $0xFFFFFFFF, %0"
        : "=r" (out3)          /* Output in 64-bit reg */
        : "i" (0x123456789ABCDEFLL)  /* Large constant */
        : "cc"
    );
    accumulator += out3;
    
    /* ASM 4: Multiple inputs/outputs with different classes */
    /* Mixed register classes increase reload pressure */
    asm volatile (
        /* Use both input operands */
        "movq %2, %0\n\t"
        "addq %3, %0\n\t"
        "movl %4, %1"
        : "=r" (out4), "=r" (out1)    /* Two outputs */
        : "r" (b),                     /* Input in reg */
          "m" (global_ll),             /* Memory operand */
          "i" (0xDEADBEEF)             /* Immediate */
        : "cc"
    );
    accumulator += out4 + out1;
    
    /* ASM 5: Floating point with conversion */
    /* Triggers reloads between different FP register classes */
    asm volatile (
        /* Double to float conversion */
        "cvtsd2ss %1, %%xmm0\n\t"
        "movss %%xmm0, %0"
        : "=m" (out5)          /* Memory output */
        : "r" (d)              /* Double in reg - may need reload */
        : "xmm0"
    );
    /* Use the result */
    out2 = (int)out5;
    accumulator += out2;
    
    /* ASM 6: Complex addressing mode */
    /* May require secondary reload for address computation */
    asm volatile (
        /* Use scaled index addressing */
        "leaq (%1,%2,2), %0"
        : "=r" (out4)
        : "r" (&global_int),   /* Address in register */
          "r" (a)              /* Scale factor in register */
        : "cc"
    );
    accumulator += out4;
    
    /* ASM 7: Vector-style operation (simulated) */
    /* Multiple constraints on same operand */
    asm volatile (
        "movq %1, %0\n\t"
        "rorq $32, %0"
        : "=r" (out3)
        : "0" (b)              /* Same as output - may need reload */
        : "cc"
    );
    accumulator += out3;
    
    return accumulator;
}

/* Main function with varied inputs */
int main(int argc, char **argv) {
    /* Use argv to create varied inputs preventing constant propagation */
    int int_val = (argc > 1) ? atoi(argv[1]) : 1000;
    long long ll_val = (argc > 2) ? atoll(argv[2]) : 5000000000LL;
    float float_val = (argc > 3) ? atof(argv[3]) : 1.2345f;
    double double_val = (argc > 4) ? atof(argv[4]) : 5.6789;
    
    /* Mix with global volatiles */
    int_val += global_int;
    ll_val ^= global_ll;
    float_val *= global_float;
    double_val /= global_double;
    
    /* Trigger the reload-intensive function */
    long long result = trigger_reloads(int_val, ll_val, float_val, double_val);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %lld\n", result);
    
    /* Additional volatile operations to ensure all asm statements run */
    volatile int check = 0;
    if (result > 0) {
        check = 1;
    }
    
    return check;
}
