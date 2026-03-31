/* reload_test.c - Test program to trigger push_reload uncovered lines */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
volatile int g_int = 42;
volatile long long g_llong = 9876543210LL;
volatile float g_float = 3.14159f;
volatile double g_double = 2.718281828459045;

/* Vector types for testing different machine modes */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function to trigger various reload scenarios */
static long long trigger_reloads(int a, long long b, float c, double d, 
                                 v4si vec_int, v4sf vec_float) {
    long long accumulator = 0;
    int out_int1, out_int2;
    long long out_llong;
    float out_float;
    double out_double;
    v4si out_vec_int;
    v4sf out_vec_float;
    
    /* ASM 1: Mixed modes with specific register constraints */
    /* This should trigger reloads with different in/out modes */
    asm volatile (
        /* Move int to output with specific constraint */
        "movl %1, %0\n\t"
        /* Use the value in a computation */
        "addl $100, %0"
        : "=r" (out_int1)          /* Output in general reg */
        : "rm" (a)                 /* Input: register or memory */
        : "cc"                     /* Clobber flags */
    );
    accumulator += out_int1;
    
    /* ASM 2: Force secondary reloads with constant pool addresses */
    /* Using 'i' constraint with complex address may need secondary reload */
    asm volatile (
        "leaq g_llong(%%rip), %0\n\t"  /* Load address of global */
        : "=r" (out_llong)             /* Output */
        :                              /* No inputs */
        : "memory"
    );
    accumulator += out_llong;
    
    /* ASM 3: Floating point with register class constraints */
    /* Mixing float/double with general purpose registers */
    asm volatile (
        "movd %1, %0\n\t"          /* Move float to general reg */
        : "=r" (out_int2)          /* Output in general reg */
        : "x" (c)                  /* Input in SSE/float reg */
        : /* No clobbers */
    );
    accumulator += out_int2;
    
    /* ASM 4: Memory constraint with complex addressing */
    /* This often requires secondary reloads */
    asm volatile (
        "movsd %1, %0\n\t"         /* Move double */
        : "=m" (out_double)        /* Memory output */
        : "m" (g_double)           /* Memory input */
        : "memory"
    );
    /* Convert double to long long for accumulator */
    accumulator += (long long)out_double;
    
    /* ASM 5: Vector operations with specific constraints */
    /* Vector moves might require special handling */
    asm volatile (
        "movdqa %1, %0\n\t"        /* Move aligned vector */
        : "=x" (out_vec_int)       /* Output in SSE reg */
        : "xm" (vec_int)           /* Input: SSE reg or memory */
        : /* No clobbers */
    );
    /* Use first element of vector */
    accumulator += out_vec_int[0];
    
    /* ASM 6: Multiple outputs with different constraints */
    /* This creates complex reload scenarios */
    int out3, out4;
    asm volatile (
        "movl %2, %0\n\t"
        "movl %3, %1\n\t"
        "addl %0, %1"
        : "=&r" (out3), "=r" (out4)  /* Two outputs, first early clobber */
        : "rm" (g_int), "rm" (a)     /* Two inputs */
        : "cc"
    );
    accumulator += out3 + out4;
    
    /* ASM 7: In/out operand with '+' constraint */
    /* These often require special reload handling */
    int inout = a;
    asm volatile (
        "addl $777, %0"
        : "+r" (inout)              /* Read-write operand */
        :                           /* No separate inputs */
        : "cc"
    );
    accumulator += inout;
    
    /* ASM 8: Force use of specific registers */
    /* Explicit register constraints can trigger reloads */
    register long long reg_ll asm("rax") = b;
    asm volatile (
        "addq $12345, %0"
        : "+r" (reg_ll)
        :
        : "cc"
    );
    accumulator += reg_ll;
    
    /* ASM 9: Large immediate that might need constant pool */
    /* Constants that don't fit in instruction need secondary reloads */
    long long big_const;
    asm volatile (
        "movq $0x123456789ABCDEF0, %0"
        : "=r" (big_const)
        :
        : /* No clobbers */
    );
    accumulator += big_const;
    
    /* ASM 10: Mixed size operations */
    /* Different modes in same asm statement */
    short out_short;
    asm volatile (
        "movw %w1, %w0\n\t"        /* 16-bit move */
        "movsxw %w0, %k0"          /* Sign extend to 32-bit */
        : "=r" (out_int1)          /* 32-bit output */
        : "rm" ((short)a)          /* 16-bit input */
        : /* No clobbers */
    );
    accumulator += out_int1;
    
    return accumulator;
}

int main(int argc, char *argv[]) {
    /* Use argv to create variable values to prevent constant propagation */
    int base = argc > 1 ? atoi(argv[1]) : 1000;
    
    /* Initialize test variables */
    int int_val = base + 1;
    long long llong_val = (long long)base * 1000LL;
    float float_val = (float)base / 3.0f;
    double double_val = (double)base * 2.5;
    
    /* Initialize vectors */
    v4si vec_int = {base, base + 1, base + 2, base + 3};
    v4sf vec_float = {float_val, float_val + 1.0f, 
                      float_val + 2.0f, float_val + 3.0f};
    
    /* Call function multiple times with different values */
    long long result1 = trigger_reloads(int_val, llong_val, 
                                       float_val, double_val,
                                       vec_int, vec_float);
    
    /* Modify values and call again */
    int_val += 100;
    long long result2 = trigger_reloads(int_val, llong_val + 1000,
                                       float_val * 2.0f, double_val / 2.0,
                                       vec_int, vec_float);
    
    printf("Result 1: %lld\n", result1);
    printf("Result 2: %lld\n", result2);
    printf("Total: %lld\n", result1 + result2);
    
    return (int)((result1 + result2) % 1000);
}
