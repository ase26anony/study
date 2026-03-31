/* reload_coverage.c - Trigger GCC reload pass with secondary reloads */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile int g_int = 12345;
volatile long long g_llong = 9876543210LL;
volatile float g_float = 3.14159f;
volatile double g_double = 2.718281828459045;
volatile int g_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

/* Function with complex inline assembly to trigger reloads */
static long long trigger_reloads(int a, long long b, float c, double d, int *ptr)
{
    long long accumulator = 0;
    int out1, out2;
    long long out3;
    float out4;
    double out5;
    int *out_ptr;
    
    /* ASM 1: Mixed types with specific register constraints */
    /* Forces reload due to mismatched constraints */
    asm volatile (
        "mov %1, %0\n\t"
        "add $42, %0"
        : "=r" (out1)          /* Output in general reg */
        : "i" (a)              /* Immediate constraint - may need reload */
        : "cc"                 /* Clobber flags */
    );
    accumulator += out1;
    
    /* ASM 2: Memory operand with complex addressing - may need secondary reload */
    /* Using 'm' constraint with array indexing */
    asm volatile (
        "movl (%1, %2, 4), %0\n\t"  /* Load array element */
        : "=r" (out2)
        : "r" (g_array), "r" (a & 7)  /* Both in registers */
        : "memory"
    );
    accumulator += out2;
    
    /* ASM 3: 64-bit operation with specific register pair constraint (x86) */
    /* May require secondary reload for 64-bit constants */
    asm volatile (
        "add %2, %1\n\t"
        "mov %1, %0"
        : "=r" (out3)
        : "0" (b), "re" (0x1234567890ABCDEFLL)  /* Constant may need reload */
        : "cc"
    );
    accumulator += out3;
    
    /* ASM 4: Floating point with memory constraints */
    /* Mixing float/double with integer registers */
    asm volatile (
        "movd %1, %0\n\t"      /* Move float to general reg (x86) */
        : "=r" (out2)
        : "x" (c)              /* SSE register constraint */
        : 
    );
    accumulator += out2;
    
    /* ASM 5: Pointer arithmetic with output constraint */
    /* May require secondary reload for address computation */
    asm volatile (
        "lea (%1, %2, 4), %0\n\t"  /* Address computation */
        : "=r" (out_ptr)
        : "r" (ptr), "r" (a)
        : 
    );
    accumulator += (long long)out_ptr;
    
    /* ASM 6: Multiple outputs with different classes */
    /* Forces different reload types */
    asm volatile (
        "mov %2, %0\n\t"
        "mov %3, %1"
        : "=r" (out1), "=m" (g_int)  /* One reg output, one memory output */
        : "r" (a), "i" (g_int)       /* Reg and immediate inputs */
        : "memory"
    );
    accumulator += out1;
    
    /* ASM 7: Vector-style operation (simulated) */
    /* Using multiple constraints on same operand */
    register long long reg_b asm("rbx") = b;  /* Suggest specific register */
    asm volatile (
        "imul %2, %1\n\t"
        "add %1, %0"
        : "+r" (accumulator)
        : "r" (reg_b), "rm" (1000LL)  /* Register or memory constraint */
        : "cc"
    );
    
    /* ASM 8: Complex constraints that likely need secondary reload */
    /* 'g' constraint allows register, memory, or immediate */
    asm volatile (
        "add %1, %0"
        : "+r" (accumulator)
        : "g" (0x7FFFFFFFFFFFFFFFLL)  /* Large constant - may need reload */
        : "cc"
    );
    
    return accumulator;
}

/* Main function with varied inputs */
int main(int argc, char *argv[])
{
    /* Use argv to create variable inputs (prevents constant propagation) */
    int base = (argc > 1) ? atoi(argv[1]) : 100;
    
    /* Initialize variables with non-constant values */
    int int_val = base + g_int;
    long long llong_val = (long long)base * g_llong;
    float float_val = (float)base * g_float;
    double double_val = (double)base * g_double;
    int *ptr_val = &g_array[base % 10];
    
    /* Call function that triggers reloads */
    long long result = trigger_reloads(int_val, llong_val, 
                                      float_val, double_val, ptr_val);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %lld\n", result);
    
    /* Additional volatile asm to ensure all paths are used */
    asm volatile ("" : : "r" (result) : "memory");
    
    return (result > 0) ? 0 : 1;
}
