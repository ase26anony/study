/* reload_test.c - Program to trigger GCC reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Helper function that forces register-based parameter passing */
__attribute__((noinline))
static long helper_func(long a, long b, long c, long d, long e, long f) {
    volatile long result = a + b * c - d / (e + 1) + f;
    return result;
}

int main(void) {
    /* Seed random number generator */
    srand(time(NULL));
    
    /* Create register pressure with many live variables */
    /* Use volatile to prevent optimization removal */
    volatile long v1 = rand() % 100;
    volatile long v2 = rand() % 100;
    volatile long v3 = rand() % 100;
    volatile long v4 = rand() % 100;
    volatile long v5 = rand() % 100;
    volatile long v6 = rand() % 100;
    volatile long v7 = rand() % 100;
    volatile long v8 = rand() % 100;
    volatile long v9 = rand() % 100;
    volatile long v10 = rand() % 100;
    volatile long v11 = rand() % 100;
    volatile long v12 = rand() % 100;
    volatile long v13 = rand() % 100;
    volatile long v14 = rand() % 100;
    volatile long v15 = rand() % 100;
    volatile long v16 = rand() % 100;
    volatile long v17 = rand() % 100;
    volatile long v18 = rand() % 100;
    volatile long v19 = rand() % 100;
    volatile long v20 = rand() % 100;
    
    /* Use explicit register variables to pin values */
    register long r12_var asm("r12") = v1 + v2;
    register long r13_var asm("r13") = v3 * v4;
    
    /* Complex expression using many variables - creates register pressure */
    long complex_expr = v1 + v2 * v3 - v4 / (v5 + 1) + 
                       v6 * v7 - v8 + v9 * v10 / (v11 + 1) -
                       v12 + v13 * v14 - v15 / (v16 + 1) +
                       v17 - v18 * v19 + v20;
    
    /* Inline assembly with fixed register constraints */
    /* This forces specific register allocation and can trigger reloads */
    asm volatile (
        /* Dummy operation that uses specific registers */
        "movq %[in1], %%rax\n\t"
        "addq %[in2], %%rax\n\t"
        "movq %%rax, %[out]\n\t"
        : [out] "=r" (complex_expr)      /* Output operand */
        : [in1] "r" (complex_expr),      /* Input operand 1 */
          [in2] "r" (r12_var)           /* Input operand 2 */
        : "rax", "rbx", "rcx", "rdx",   /* Clobber specific registers */
          "rsi", "rdi", "r8", "r9",     /* More clobbers for pressure */
          "r10", "r11", "cc", "memory"  /* Memory clobber prevents reordering */
    );
    
    /* Another inline asm with mismatched constraints */
    long temp;
    asm volatile (
        "movl %1, %0\n\t"
        : "=r" (temp)        /* Output in general register */
        : "m" (v1)           /* Input from memory - may force reload */
        : 
    );
    
    /* Force address reloads with complex memory addressing */
    long array[100];
    for (int i = 0; i < 20; i++) {
        /* Complex addressing with multiple variables */
        array[v1 + v2 * i - v3 / (v4 + 1)] = v5 + v6 * i;
    }
    
    /* Mix different data types to cause mode conversions */
    {
        char c1 = v1 & 0xFF;
        short s1 = v2 & 0xFFFF;
        int i1 = v3;
        long l1 = v4;
        
        /* Operations requiring mode conversions */
        long mixed = c1 + s1 * i1 - l1 / (c1 + 1);
        complex_expr += mixed;
    }
    
    /* Call helper function - forces parameter passing in registers */
    long func_result = helper_func(v7, v8, v9, v10, v11, v12);
    
    /* Use volatile variable in condition to prevent elimination */
    volatile int condition = (complex_expr > 1000);
    if (condition) {
        /* More complex operations in conditional path */
        v1 = v2 + v3 * v4 - v5 / (v6 + 1) + v7 - v8 * v9;
    }
    
    /* Final computation using all variables */
    long final_result = 
        v1 + v2 - v3 * v4 + v5 / (v6 + 1) -
        v7 + v8 * v9 - v10 / (v11 + 1) +
        v12 - v13 * v14 + v15 / (v16 + 1) -
        v17 + v18 * v19 - v20 / (func_result + 1) +
        complex_expr + r12_var - r13_var;
    
    /* Use the result to prevent optimization */
    printf("Result: %ld\n", final_result);
    
    return (final_result > 0) ? 0 : 1;
}

/* Additional function to create more register pressure */
__attribute__((noinline))
static void more_pressure(void) {
    /* Local variables with different lifetimes */
    long a1 = 1, a2 = 2, a3 = 3, a4 = 4, a5 = 5;
    long b1 = 6, b2 = 7, b3 = 8, b4 = 9, b5 = 10;
    long c1 = 11, c2 = 12, c3 = 13, c4 = 14, c5 = 15;
    
    /* Complex nested expressions */
    long result1 = (a1 * a2) + (a3 - a4) / (a5 + 1);
    long result2 = (b1 + b2) * (b3 - b4) / (b5 + 1);
    long result3 = (c1 - c2) + (c3 * c4) / (c5 + 1);
    
    /* Inline asm with fixed register */
    asm volatile (
        "movq %1, %%r14\n\t"
        "addq %2, %%r14\n\t"
        : "=r" (result1)
        : "r" (result2), "r" (result3)
        : "r14"
    );
    
    volatile long sink = result1 + result2 + result3;
    (void)sink;  /* Use variable to prevent optimization */
}
