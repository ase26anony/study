/* reload_coverage.c - Test program to cover push_reload initialization */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
volatile int global_int = 42;
volatile long long global_ll = 0x123456789ABCDEFLL;
volatile float global_float = 3.14159f;
volatile double global_double = 2.718281828459045;

/* Function with complex inline assembly to trigger reloads */
static long long trigger_reloads(int a, long long b, float c, double d) {
    int out1, out2;
    long long out3;
    float out4;
    double out5;
    long long accumulator = 0;
    
    /* ASM 1: Mixed modes with specific register constraints */
    /* Forces reload due to mismatched constraints */
    asm volatile (
        "movl %1, %0\n\t"
        : "=r" (out1)          /* Output in general reg */
        : "i" (0x7FFFFFFF)     /* Immediate that might need reload */
        : "cc"
    );
    accumulator += out1;
    
    /* ASM 2: Requires secondary reload - address computation */
    /* Using 'm' constraint with complex addressing */
    asm volatile (
        "addl %1, %0\n\t"
        : "+r" (out1)          /* Read-write operand */
        : "m" (global_int)     /* Memory operand needing address reload */
        : "cc"
    );
    accumulator += out1;
    
    /* ASM 3: Different input/output modes with register pressure */
    /* Creates need for secondary reloads */
    asm volatile (
        "movq %1, %0\n\t"
        : "=r" (out3)          /* 64-bit output */
        : "r" (b), "m" (global_ll)  /* Register and memory inputs */
        : "cc"
    );
    accumulator += out3;
    
    /* ASM 4: Floating point with general register constraints */
    /* Forces reload between register classes */
    asm volatile (
        "movd %1, %0\n\t"
        : "=r" (out2)          /* Integer output from float */
        : "x" (c)              /* SSE/XMM register input */
        : "cc"
    );
    accumulator += out2;
    
    /* ASM 5: Multiple outputs with specific clobbers */
    /* Increases register pressure */
    asm volatile (
        "movl %%eax, %0\n\t"
        "movl %%ebx, %1\n\t"
        : "=r" (out1), "=r" (out2)
        : "a" (a), "b" (global_int)  /* Specific register constraints */
        : "cc"
    );
    accumulator += out1 + out2;
    
    /* ASM 6: Memory operand with offset that needs computation */
    /* Likely requires secondary reload for address */
    asm volatile (
        "movsd %1, %0\n\t"
        : "=x" (out5)          /* XMM output */
        : "m" (global_double)  /* Memory with potential complex address */
        : "cc"
    );
    accumulator += (long long)out5;
    
    /* ASM 7: In/out operand with different modes */
    /* Tests inmode/outmode initialization */
    asm volatile (
        "cvtsi2sd %1, %0\n\t"
        : "=x" (out5)          /* Double output */
        : "r" (a)              /* Integer input */
        : "cc"
    );
    accumulator += (long long)out5;
    
    /* ASM 8: Vector-type operation suggestion */
    /* Using larger types to stress reload system */
    typedef long long v2ll __attribute__((vector_size(16)));
    v2ll vec_in = {b, b + 1};
    v2ll vec_out;
    
    asm volatile (
        "paddq %1, %0\n\t"
        : "=x" (vec_out)
        : "x" (vec_in), "m" (global_ll)
        : "cc"
    );
    accumulator += vec_out[0];
    
    return accumulator;
}

int main(int argc, char **argv) {
    /* Use argv to create variant inputs preventing constant propagation */
    int input_int = argc > 1 ? atoi(argv[1]) : 100;
    long long input_ll = argc > 2 ? atoll(argv[2]) : 1000LL;
    float input_float = argc > 3 ? (float)atof(argv[3]) : 1.234f;
    double input_double = argc > 4 ? atof(argv[4]) : 5.678;
    
    /* Modify globals to affect memory operands */
    global_int = input_int * 2;
    global_ll = input_ll + 1;
    global_float = input_float * 2.0f;
    global_double = input_double * 3.0;
    
    long long result = trigger_reloads(input_int, input_ll, 
                                       input_float, input_double);
    
    printf("Result: %lld\n", result);
    return (int)(result % 256);
}
