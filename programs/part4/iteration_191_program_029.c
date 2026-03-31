/* reload_test.c - Test program to trigger GCC reload pass uncovered lines */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    /* This forces reloads due to mismatched constraints */
    asm volatile (
        /* Move int to output with 'r' constraint, but input might need secondary reload */
        "movl %1, %0\n\t"
        /* Use the value in a computation */
        "addl $100, %0"
        : "=r" (out1)          /* Output in general register */
        : "i" (a + 256)        /* Immediate that might need secondary reload on some arches */
        : "cc"                 /* Clobber flags */
    );
    accumulator += out1;
    
    /* ASM 2: Memory operand with complex addressing that requires reload */
    /* The address calculation might need secondary reload */
    asm volatile (
        "movq (%1), %0\n\t"    /* Load from memory address */
        "addq %2, %0"          /* Add another value */
        : "=&r" (out3)         /* Early clobber output */
        : "r" (&g_array[a % 10]),  /* Address that might need computation */
          "ri" (b)             /* Register or immediate */
        : "memory", "cc"
    );
    accumulator += out3;
    
    /* ASM 3: Floating point with general register constraints */
    /* Forces reload between float regs and general regs */
    asm volatile (
        /* Move float through integer register (requires secondary reload) */
        "movd %1, %0\n\t"      /* Move float bits to general register */
        "addl $1, %0\n\t"      /* Modify in integer domain */
        "movd %0, %2"          /* Move back to float register */
        : "=r" (out2), "=t" (out4)
        : "0" (0), "1" (c)     /* Input float in floating register */
        : "cc"
    );
    /* Use the result */
    accumulator += (long long)(out4 * 1000);
    
    /* ASM 4: Double with memory constraint and specific register */
    /* Complex addressing mode that might need secondary reload */
    asm volatile (
        "movsd %1, %0\n\t"     /* Load double */
        "addsd %2, %0"         /* Add another double */
        : "=x" (out5)          /* Output in SSE register */
        : "m" (g_double),      /* Memory operand */
          "x" (d)              /* SSE register input */
        : 
    );
    accumulator += (long long)out5;
    
    /* ASM 5: Pointer arithmetic with specific constraints */
    /* Forces address reload with possible secondary reload */
    asm volatile (
        "leaq (%1, %2, 4), %0\n\t"  /* Complex address calculation */
        : "=r" (out_ptr)
        : "r" (ptr), "r" (a)    /* Both in registers */
        : 
    );
    
    /* Use the pointer to prevent optimization */
    if (out_ptr) {
        accumulator += *out_ptr;
    }
    
    /* ASM 6: Multiple outputs with early clobber */
    /* Creates pressure on register allocator */
    asm volatile (
        "imull %2, %0\n\t"     /* Multiply */
        "movl %0, %1\n\t"      /* Copy to second output */
        "addl %3, %1"
        : "=&r" (out1), "=r" (out2)  /* Both early clobber */
        : "r" (a), "i" (g_int)       /* Register and immediate */
        : "cc"
    );
    accumulator += out1 + out2;
    
    /* ASM 7: 64-bit operation with 32-bit parts */
    /* Might trigger mode-specific reloads */
    {
        int out_lo, out_hi;
        asm volatile (
            "movl %%eax, %0\n\t"     /* Low part */
            "movl %%edx, %1\n\t"     /* High part */
            : "=r" (out_lo), "=r" (out_hi)
            : "A" (b)                /* Input in edx:eax */
            : 
        );
        accumulator += ((long long)out_hi << 32) | out_lo;
    }
    
    return accumulator;
}

/* Another function with different reload patterns */
static int more_reloads(int x, int y)
{
    int result;
    long long temp;
    
    /* ASM with 'm' constraint and immediate that might need secondary reload */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl (%%rbx), %%eax\n\t"   /* Memory reference via register */
        "movl %%eax, %0"
        : "=r" (result)
        : "i" (x * 4096),           /* Large immediate */
          "b" (&g_int)              /* Specific register constraint */
        : "%eax", "memory", "cc"
    );
    
    /* ASM with output in specific register pair */
    asm volatile (
        "mulq %2\n\t"               /* 64-bit multiply */
        : "=A" (temp)               /* Output in edx:eax */
        : "a" ((long long)x),       /* Input in eax */
          "r" ((long long)y)        /* Input in any register */
        : "%edx", "cc"
    );
    
    return result + (int)temp;
}

int main(int argc, char **argv)
{
    long long total = 0;
    int i;
    
    /* Use argv to create variable inputs preventing constant propagation */
    int base = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Initialize test variables with non-constant values */
    int int_val = base + g_int;
    long long llong_val = (long long)base * g_llong;
    float float_val = (float)base * g_float;
    double double_val = (double)base * g_double;
    
    /* Call reload-intensive functions multiple times */
    for (i = 0; i < 3; i++) {
        total += trigger_reloads(int_val + i, 
                                llong_val + i, 
                                float_val + i,
                                double_val + i,
                                g_array);
        
        total += more_reloads(int_val + i * 2, 
                             (int)llong_val + i * 3);
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %lld\n", total);
    
    /* Also use global vars to prevent optimization */
    g_int = (int)total;
    g_llong = total;
    
    return (int)(total % 1000);
}
