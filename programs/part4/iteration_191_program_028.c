/* Test program to trigger push_reload with secondary reloads in GCC reload pass */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
volatile int g_int = 42;
volatile long long g_llong = 123456789012345LL;
volatile float g_float = 3.14159f;
volatile double g_double = 2.718281828459045;
volatile int g_array[10] = {0,1,2,3,4,5,6,7,8,9};

/* Vector type for testing vector reloads */
typedef int v4si __attribute__((vector_size(16)));

/* Function with complex inline assembly to trigger reloads */
static long long trigger_reloads(int a, long long b, float c, double d, v4si v)
{
    long long accumulator = 0;
    int out_int1, out_int2;
    long long out_llong;
    float out_float;
    double out_double;
    v4si out_vec;
    
    /* Assembly 1: Mixed types with specific register constraints */
    /* This should trigger reloads with secondary reload info */
    asm volatile (
        /* Move int to output with 'r' constraint - may need secondary reload */
        "movl %1, %0\n\t"
        /* Use the value in computation */
        "addl $100, %0"
        : "=r" (out_int1)          /* Output in general reg, may need secondary */
        : "i" (g_int)              /* Immediate constraint that may not fit */
        : "cc"                     /* Clobber flags */
    );
    accumulator += out_int1;
    
    /* Assembly 2: Memory operand with complex addressing */
    /* May require secondary reload for address computation */
    asm volatile (
        /* Load from memory with complex address */
        "movq (%q1), %0\n\t"       /* q modifier for DImode */
        /* Do some operation */
        "addq $0x12345678, %0"
        : "=r" (out_llong)         /* Output in register */
        : "r" (&g_array[g_int])    /* Address that may need computation */
        : "memory", "cc"
    );
    accumulator += out_llong;
    
    /* Assembly 3: Floating point with general register constraints */
    /* May require secondary reloads for float<->int moves */
    asm volatile (
        /* Move float through integer register (triggers reload) */
        "movd %1, %0\n\t"          /* Move float bits to general reg */
        "addl $0x40000000, %0\n\t" /* Modify float representation */
        "movd %0, %1"              /* Move back */
        : "=r" (out_int2), "+r" (out_float)
        : "0" (0), "1" (c)         /* Input float in 'r' constraint */
        : "cc"
    );
    accumulator += out_int2;
    
    /* Assembly 4: Multiple outputs with different modes */
    /* Tests inmode/outmode initialization */
    asm volatile (
        /* Operations with different sized operands */
        "mov %2, %0\n\t"           /* 32-bit move */
        "mov %3, %1\n\t"           /* 64-bit move */
        "addl $1, %0\n\t"
        "addq $1, %1"
        : "=r" (out_int1), "=r" (out_llong)  /* Different output modes */
        : "r" (a), "r" (b)                   /* Different input modes */
        : "cc"
    );
    accumulator += out_int1 + out_llong;
    
    /* Assembly 5: Vector operations with potential reloads */
    /* Vector moves might need secondary reloads */
    asm volatile (
        /* Vector operation */
        "movdqa %1, %0\n\t"        /* Aligned vector move */
        "paddd %2, %0"             /* Vector add */
        : "=x" (out_vec)           /* XMM register constraint */
        : "x" (v), "xm" (v)        /* Mix of register and memory constraints */
        : "cc"
    );
    
    /* Use vector result to affect accumulator */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += out_vec[i];
    }
    accumulator += sum;
    
    /* Assembly 6: Complex constraints that force reloads */
    /* 'a' constraint for specific register */
    asm volatile (
        "mov %1, %%eax\n\t"        /* Use specific register */
        "imul %2, %%eax\n\t"
        "mov %%eax, %0"
        : "=r" (out_int1)          /* Output */
        : "r" (a), "r" (g_int)     /* Inputs */
        : "%eax", "cc"             /* Clobber eax */
    );
    accumulator += out_int1;
    
    /* Assembly 7: Memory output with register input */
    /* Tests out_reg initialization */
    asm volatile (
        "mov %1, %0"               /* Store to memory */
        : "=m" (g_array[2])        /* Memory output */
        : "r" (a)                  /* Register input */
        : "memory"
    );
    accumulator += g_array[2];
    
    return accumulator;
}

int main(int argc, char **argv)
{
    /* Use argv to create variant inputs */
    int base = argc > 1 ? atoi(argv[1]) : 100;
    
    /* Initialize test variables with non-constant values */
    int int_val = base + 1;
    long long llong_val = (long long)base * 1000LL;
    float float_val = (float)base / 10.0f;
    double double_val = (double)base / 3.0;
    
    /* Initialize vector */
    v4si vec_val = {base, base+1, base+2, base+3};
    
    /* Call function multiple times with different values */
    long long result = 0;
    for (int i = 0; i < 3; i++) {
        result += trigger_reloads(
            int_val + i,
            llong_val + i,
            float_val + i,
            double_val + i,
            vec_val
        );
        
        /* Modify values to create different reload scenarios */
        int_val += 100;
        llong_val += 1000000LL;
    }
    
    printf("Result: %lld\n", result);
    return (int)(result % 256);
}
