/* reload_coverage.c
 * Designed to trigger push_reload with full parameter initialization,
 * including secondary reload fields.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variables to prevent optimization */
volatile int global_int = 42;
volatile long long global_ll = 0x123456789ABCDEF0LL;
volatile float global_float = 3.14159f;
volatile double global_double = 2.718281828459045;
volatile char global_buffer[64] = "Test string for addressing modes";

/* Function with complex inline assembly to trigger reloads */
static long long trigger_reloads(int a, long long b, float c, double d, char *ptr)
{
    /* Local variables for output operands */
    int out1, out2;
    long long out3, out4;
    float out5;
    double out6;
    long long accumulator = 0;
    
    /* Force register pressure by using many variables */
    register int r1 asm("r15") = a;  /* Suggest specific register */
    register long long r2 asm("r14") = b;
    
    /* ASM 1: Mixed modes with specific register constraints
     * This should trigger reloads due to register class mismatches */
    asm volatile (
        "movl %[in1], %[out1]\n\t"
        "addl %[in2], %[out1]\n\t"
        : [out1] "=r" (out1)        /* Output in general reg */
        : [in1] "r" (a),            /* Input in general reg */
          [in2] "i" (global_int)    /* Immediate that may need reload */
        : "cc"                      /* Clobber flags */
    );
    accumulator += out1;
    
    /* ASM 2: Memory addressing with complex constraints
     * Likely to require secondary reloads for address computation */
    asm volatile (
        "movq (%[addr]), %[out3]\n\t"
        "addq %[imm], %[out3]\n\t"
        : [out3] "=r" (out3)        /* Output in 64-bit reg */
        : [addr] "r" (ptr),         /* Base address in register */
          [imm] "i" (0x1000)        /* Large immediate */
        : "memory"
    );
    accumulator += out3;
    
    /* ASM 3: Floating point with general register constraints
     * May require secondary reloads on some architectures */
    asm volatile (
        "movd %[in_flt], %[out_int]\n\t"  /* Move float to int reg */
        "addl $1, %[out_int]\n\t"
        : [out_int] "=r" (out2)     /* Integer output */
        : [in_flt] "r" (*(int*)&c)  /* Float bit pattern in int reg */
        : "cc"
    );
    accumulator += out2;
    
    /* ASM 4: Multiple outputs with different modes
     * Forces different inmode/outmode initialization */
    asm volatile (
        "movl %[in_a], %[out_a]\n\t"
        "movq %[in_b], %[out_b]\n\t"
        : [out_a] "=r" (out1),      /* 32-bit output */
          [out_b] "=r" (out4)       /* 64-bit output */
        : [in_a] "r" (a),           /* 32-bit input */
          [in_b] "r" (b)            /* 64-bit input */
        : /* No clobbers */
    );
    accumulator += out1 + out4;
    
    /* ASM 5: Using suggested registers that conflict
     * May require reload due to register allocation pressure */
    asm volatile (
        "addq %[reg1], %[reg2]\n\t"
        "movq %[reg2], %[out]\n\t"
        : [out] "=r" (out4)
        : [reg1] "r" (r1),          /* Suggested r15 */
          [reg2] "r" (r2)           /* Suggested r14 */
        : /* Clobber suggested registers implicitly */
    );
    accumulator += out4;
    
    /* ASM 6: Vector-style operation (simulated)
     * Uses multiple constraints that may need secondary reloads */
    {
        long long tmp1 = b, tmp2 = global_ll;
        asm volatile (
            "movq %[src1], %[dst1]\n\t"
            "addq %[src2], %[dst1]\n\t"
            : [dst1] "=&r" (out3)   /* Early clobber */
            : [src1] "rm" (tmp1),   /* Register or memory */
              [src2] "rm" (tmp2)    /* Register or memory */
            : "cc"
        );
        accumulator += out3;
    }
    
    /* ASM 7: Complex addressing mode
     * May require secondary reload for address calculation */
    asm volatile (
        "leaq (%[base], %[index], 4), %[out]\n\t"
        : [out] "=r" (out4)
        : [base] "r" (ptr),
          [index] "r" (a & 0xF)     /* Restricted range */
        : /* No clobbers */
    );
    accumulator += out4;
    
    return accumulator;
}

/* Wrapper to ensure variables are used */
static long long test_function(int seed)
{
    /* Create varied inputs */
    int int_val = seed * 3;
    long long ll_val = (long long)seed * 1000000;
    float float_val = (float)seed / 7.0f;
    double double_val = (double)seed / 11.0;
    char *ptr = (char*)global_buffer + (seed % 32);
    
    /* Call the reload trigger multiple times */
    long long total = 0;
    total += trigger_reloads(int_val, ll_val, float_val, double_val, ptr);
    total += trigger_reloads(int_val + 1, ll_val + 1, 
                            float_val + 1.0f, double_val + 1.0, 
                            ptr + 1);
    
    return total;
}

int main(int argc, char **argv)
{
    /* Use argv for variability to prevent constant propagation */
    int seed = 1;
    if (argc > 1) {
        seed = atoi(argv[1]);
        if (seed == 0) seed = 1;
    }
    
    /* Run the test */
    long long result = test_function(seed);
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %lld\n", result);
    
    /* Also use global variables to keep them alive */
    printf("Globals: %d %lld %f %f %s\n", 
           global_int, global_ll, 
           global_float, global_double,
           global_buffer);
    
    return (result > 0) ? 0 : 1;
}
