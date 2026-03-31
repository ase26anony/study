/* reload_coverage.c - Test program to cover push_reload initialization */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
volatile int g_int = 42;
volatile long long g_llong = 9876543210LL;
volatile float g_float = 3.14159f;
volatile double g_double = 2.718281828459045;

/* Function with complex inline assembly to trigger reloads */
static long long trigger_reloads(int a, long long b, float c, double d) {
    int out1, out2;
    long long out3;
    float out4;
    double out5;
    long long accumulator = 0;
    
    /* ASM 1: Mixed integer types with specific register constraints */
    /* This should trigger reloads with secondary reload info */
    asm volatile (
        /* Move with potential need for secondary reload */
        "movl %1, %0\n\t"
        "addl $100, %0"
        : "=r" (out1)          /* Output in general reg */
        : "i" (0x12345678)     /* Immediate that might need reload */
        : "cc"                 /* Clobber flags */
    );
    accumulator += out1;
    
    /* ASM 2: Floating point with memory constraints */
    /* Forces reload between FP and general registers */
    asm volatile (
        "movss %1, %0\n\t"
        "addss %2, %0"
        : "=x" (out4)          /* Output in SSE register */
        : "m" (c),             /* Input from memory (force reload) */
          "x" (g_float)        /* Input in SSE register */
        : /* No clobbers */
    );
    accumulator += (long long)out4;
    
    /* ASM 3: 64-bit operations with specific constraints */
    /* May require secondary reloads for 64-bit constants */
    asm volatile (
        "movq %1, %0\n\t"
        "addq %2, %0"
        : "=r" (out3)          /* Output in 64-bit register */
        : "r" (b),             /* Input in register */
          "i" (0x7FFFFFFFFFFFFFFFLL)  /* Large immediate */
        : "cc"
    );
    accumulator += out3;
    
    /* ASM 4: Mixed mode - double with integer conversion */
    /* Triggers different inmode/outmode initialization */
    asm volatile (
        "cvtsd2si %1, %0\n\t"  /* Convert double to int */
        : "=r" (out2)          /* Integer output */
        : "x" (d)              /* Double input in SSE */
        : /* No clobbers */
    );
    accumulator += out2;
    
    /* ASM 5: Complex addressing mode with multiple clobbers */
    /* Increases reload pressure */
    asm volatile (
        "leaq (%1,%2,1), %0\n\t"
        : "=r" (out3)
        : "r" (&g_int),        /* Address that may need reload */
          "r" (a)              /* Scaled index */
        : "rax", "rcx"         /* Force specific clobbers */
    );
    accumulator += out3;
    
    /* ASM 6: Vector-style operation (simulated) */
    /* Tests different register classes */
    {
        typedef long long v2ll __attribute__((vector_size(16)));
        v2ll v_in = {b, b + 1};
        v2ll v_out;
        
        asm volatile (
            "movdqa %1, %0\n\t"
            "paddq %2, %0"
            : "=x" (v_out)
            : "x" (v_in),
              "m" (g_llong)    /* Memory operand forcing reload */
            : /* No clobbers */
        );
        accumulator += v_out[0] + v_out[1];
    }
    
    return accumulator;
}

int main(int argc, char **argv) {
    /* Use argv to create non-constant inputs */
    int arg_int = argc > 1 ? atoi(argv[1]) : 100;
    long long arg_llong = argc > 2 ? atoll(argv[2]) : 1000LL;
    float arg_float = argc > 3 ? atof(argv[3]) : 1.5f;
    double arg_double = argc > 4 ? atof(argv[4]) : 2.5;
    
    /* Call the reload trigger function */
    long long result = trigger_reloads(
        arg_int + g_int,
        arg_llong + g_llong,
        arg_float + g_float,
        arg_double + g_double
    );
    
    printf("Result: %lld\n", result);
    return (int)(result % 256);
}
