/* reload_test.c - Test program to trigger GCC reload pass uncovered lines */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
volatile int g_int = 42;
volatile long long g_llong = 0x123456789ABCDEFLL;
volatile float g_float = 3.14159f;
volatile double g_double = 2.718281828459045;

/* Function with complex inline assembly to trigger reloads */
static long long trigger_reloads(int a, long long b, float c, double d) {
    int out1, out2;
    long long out3;
    float out4;
    double out5;
    long long accumulator = 0;
    
    /* ASM 1: Mixed modes with specific register constraints requiring reloads */
    /* This should trigger secondary reloads due to constant pool addressing */
    asm volatile (
        /* Move 32-bit int with specific register constraint */
        "movl %1, %0\n\t"
        /* Use %k0 modifier for 32-bit register name */
        : "=r"(out1)          /* Output in general reg, may need secondary reload */
        : "i"(g_int)          /* Input as immediate (may go to constant pool) */
        : /* no clobbers */
    );
    accumulator += out1;
    
    /* ASM 2: 64-bit operation with mismatched constraints */
    /* Using 'a' constraint (rax) which may require secondary reload */
    asm volatile (
        "movq %2, %0\n\t"     /* Move 64-bit value */
        "addq %1, %0\n\t"     /* Add another 64-bit value */
        : "=a"(out3)          /* Output must be in rax register */
        : "r"(b),             /* Input in any general register */
          "m"(g_llong)        /* Input from memory (may need address reload) */
        : "cc"                /* Clobbers condition codes */
    );
    accumulator += out3;
    
    /* ASM 3: Floating point with mixed precision requiring reloads */
    /* This may trigger secondary reloads for floating point registers */
    asm volatile (
        "movss %1, %0\n\t"    /* Move single precision float */
        "cvtss2sd %0, %0\n\t" /* Convert to double precision */
        : "=x"(out5)          /* Output in SSE register (xmm0-xmm15) */
        : "m"(c)              /* Input from memory (float variable) */
        : /* no clobbers */
    );
    /* Use the result to prevent elimination */
    accumulator += (long long)out5;
    
    /* ASM 4: Complex addressing mode that may need secondary reload */
    /* Using multiple constraints that conflict */
    asm volatile (
        "imull %1, %0\n\t"    /* 32-bit multiply */
        : "=r"(out2)          /* Output in general register */
        : "r"(a),             /* Input in general register */
          "i"(256)            /* Immediate that might need reload */
        : "cc", "eax", "edx"  /* Clobber eax, edx for imul */
    );
    accumulator += out2;
    
    /* ASM 5: Vector-style operation with specific constraints */
    /* Using 'x' constraint for SSE registers */
    {
        double temp_in = d + 1.0;
        asm volatile (
            "movsd %1, %0\n\t"
            "addsd %2, %0\n\t"
            : "=x"(out5)      /* Must be in SSE register */
            : "x"(temp_in),   /* Input must be in SSE register */
              "m"(g_double)   /* Memory operand needing address reload */
            : /* no clobbers */
        );
        accumulator += (long long)out5;
    }
    
    /* ASM 6: Multiple outputs with conflicting constraints */
    {
        int out6, out7;
        asm volatile (
            "movl %2, %0\n\t"
            "leal (%0, %1, 2), %1\n\t"
            : "=&r"(out6), "=r"(out7)  /* Early clobber on out6 */
            : "r"(a), "r"(out1)        /* Both inputs in registers */
            : "cc"
        );
        accumulator += out6 + out7;
    }
    
    return accumulator;
}

int main(int argc, char **argv) {
    /* Use argv to create variable inputs preventing constant propagation */
    int input_int = argc > 1 ? atoi(argv[1]) : 100;
    long long input_llong = argc > 2 ? atoll(argv[2]) : 1000LL;
    float input_float = argc > 3 ? atof(argv[3]) : 1.234f;
    double input_double = argc > 4 ? atof(argv[4]) : 5.678;
    
    /* Call the function that triggers reloads */
    long long result = trigger_reloads(
        input_int, 
        input_llong, 
        input_float, 
        input_double
    );
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %lld\n", result);
    
    return (int)(result % 256);
}
