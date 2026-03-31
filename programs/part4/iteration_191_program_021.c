/* reload_test.c - Trigger GCC reload pass with complex inline assembly */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
volatile int g_int = 12345;
volatile long long g_llong = 9876543210LL;
volatile float g_float = 3.14159f;
volatile double g_double = 2.718281828459045;

/* Vector type for potential SIMD reloads */
typedef int v4si __attribute__((vector_size(16)));

/* Function with multiple inline asm statements requiring reloads */
static long long trigger_reloads(int a, long long b, float c, double d, v4si v)
{
    long long accumulator = 0;
    int out1, out2;
    long long out3;
    float out4;
    double out5;
    v4si out6;
    
    /* ASM 1: Mixed types with specific register constraints */
    /* Forces reload due to "r" constraint on memory-like operand */
    asm volatile (
        "movl %1, %0\n\t"
        "addl $100, %0"
        : "=r" (out1)          /* Output in general reg */
        : "m" (g_int)          /* Input from memory - may need secondary reload */
        : "cc"                 /* Clobber flags */
    );
    accumulator += out1;
    
    /* ASM 2: Requires secondary reload for constant pool address */
    /* Using "i" constraint with complex address calculation */
    asm volatile (
        "movq %1, %0\n\t"
        "subq $1000, %0"
        : "=r" (out3)          /* Output */
        : "i" (&g_llong)       /* Constant address - may need secondary reload */
        : "cc"
    );
    accumulator += out3;
    
    /* ASM 3: Floating point with mismatched constraints */
    /* "r" constraint on float may require secondary reload */
    asm volatile (
        "movss %1, %0\n\t"
        "mulss %2, %0"
        : "=x" (out4)          /* Output in SSE register */
        : "r" (c),             /* Float in general reg - needs secondary reload */
          "x" (g_float)        /* Float in SSE reg */
        : "cc"
    );
    accumulator += (long long)out4;
    
    /* ASM 4: Double with specific register class constraint */
    asm volatile (
        "movsd %1, %0\n\t"
        "addsd %2, %0"
        : "=x" (out5)          /* Output in SSE register */
        : "x" (d),             /* Input in SSE register */
          "m" (g_double)       /* Memory operand - may need reload */
        : "cc"
    );
    accumulator += (long long)out5;
    
    /* ASM 5: Vector operations with multiple constraints */
    /* Complex constraints to trigger various reload types */
    asm volatile (
        "movdqa %1, %0\n\t"
        "paddd %2, %0"
        : "=x" (out6)          /* Output in XMM register */
        : "x" (v),             /* Input in XMM register */
          "m" (v)              /* Same input also from memory - interesting case */
        : "cc"
    );
    accumulator += out6[0] + out6[1];
    
    /* ASM 6: Multiple outputs with tied operands */
    /* Creates complex reload scenario with output/input overlap */
    asm volatile (
        "movl %2, %0\n\t"
        "movl %0, %1\n\t"
        "addl $42, %1"
        : "=&r" (out1), "=r" (out2)  /* Two outputs, first is earlyclobber */
        : "m" (a)                     /* Input from memory */
        : "cc"
    );
    accumulator += out1 + out2;
    
    /* ASM 7: In/out operand with "+r" constraint */
    /* Tests reload initialization for in-out operands */
    {
        int inout = a;
        asm volatile (
            "addl $777, %0"
            : "+r" (inout)     /* Read-write operand */
            :                  /* No pure inputs */
            : "cc"
        );
        accumulator += inout;
    }
    
    /* ASM 8: Complex addressing mode requiring secondary reload */
    /* Displacement that might not fit in instruction encoding */
    {
        struct large_struct {
            char data[1000000];
        } *ptr = 0;
        
        asm volatile (
            "movl $999, %%eax\n\t"
            "movl %%eax, %0"
            : "=m" (*(int*)((char*)ptr + 65536))  /* Large displacement */
            :                                     /* No inputs */
            : "eax", "cc"
        );
    }
    
    return accumulator;
}

/* Main function with varied inputs */
int main(int argc, char **argv)
{
    /* Use argv to create variable inputs (prevents constant propagation) */
    int int_val = argc > 1 ? atoi(argv[1]) : 1000;
    long long llong_val = argc > 2 ? atoll(argv[2]) : 5000LL;
    float float_val = argc > 3 ? atof(argv[3]) : 1.234f;
    double double_val = argc > 4 ? atof(argv[4]) : 5.678;
    
    /* Initialize vector */
    v4si vec_val = { int_val, int_val + 1, int_val + 2, int_val + 3 };
    
    /* Call the reload-intensive function */
    long long result = trigger_reloads(int_val, llong_val, float_val, 
                                       double_val, vec_val);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %lld\n", result);
    
    /* Additional volatile asm to ensure all asm statements are considered */
    asm volatile ("" : : : "memory");
    
    return (result > 0) ? 0 : 1;
}
