/* reload_trigger.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Helper function that forces register-based parameter passing */
__attribute__((noinline))
static long helper_func(long a, long b, long c, long d, long e, long f) {
    volatile long result = a + b - c * d / (e + f + 1);
    return result;
}

int main(void) {
    /* Seed random number generator */
    srand(time(NULL));
    
    /* Create register pressure with many live variables */
    /* Use volatile to prevent optimization removal */
    volatile long v1 = rand() % 100 + 1;
    volatile long v2 = rand() % 100 + 1;
    volatile long v3 = rand() % 100 + 1;
    volatile long v4 = rand() % 100 + 1;
    volatile long v5 = rand() % 100 + 1;
    volatile long v6 = rand() % 100 + 1;
    volatile long v7 = rand() % 100 + 1;
    volatile long v8 = rand() % 100 + 1;
    volatile long v9 = rand() % 100 + 1;
    volatile long v10 = rand() % 100 + 1;
    volatile long v11 = rand() % 100 + 1;
    volatile long v12 = rand() % 100 + 1;
    volatile long v13 = rand() % 100 + 1;
    volatile long v14 = rand() % 100 + 1;
    volatile long v15 = rand() % 100 + 1;
    volatile long v16 = rand() % 100 + 1;
    volatile long v17 = rand() % 100 + 1;
    volatile long v18 = rand() % 100 + 1;
    volatile long v19 = rand() % 100 + 1;
    volatile long v20 = rand() % 100 + 1;
    
    /* Use explicit register variables to pin values */
    register long r12_var asm("r12") = v1 + v2;
    register long r13_var asm("r13") = v3 * v4;
    register long r14_var asm("r14") = v5 - v6;
    register long r15_var asm("r15") = v7 / (v8 + 1);
    
    /* Complex expression using most variables - creates register pressure */
    long complex_expr = 
        v1 + v2 * v3 - v4 / (v5 + 1) +
        v6 * v7 - v8 + v9 / (v10 + 2) +
        v11 - v12 * v13 + v14 / (v15 + 3) -
        v16 + v17 * v18 - v19 / (v20 + 4);
    
    /* Inline assembly with fixed register constraints and clobbers */
    /* This forces the compiler to work around specific register usage */
    asm volatile (
        "/* Begin reload-triggering assembly */\n\t"
        "movq %[in1], %%rax\n\t"      /* Force use of rax */
        "addq %[in2], %%rax\n\t"
        "movq %%rax, %[out]\n\t"
        "/* Clobber multiple registers */\n\t"
        : [out] "=r" (complex_expr)   /* Output operand */
        : [in1] "r" (r12_var),        /* Input operands */
          [in2] "r" (r13_var)
        : "rax", "rbx", "rcx", "rdx", "memory", "cc"
    );
    
    /* More inline assembly with mismatched constraints */
    long temp1, temp2;
    asm volatile (
        "movl %1, %0\n\t"
        : "=r" (temp1)                /* General purpose reg constraint */
        : "m" (v9)                    /* Memory constraint - mismatch! */
        : 
    );
    
    /* Force address reloads with complex array indexing */
    long arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * i;
    }
    
    /* Complex addressing mode that may require reloads */
    long array_access = 
        arr[v1 + v2 * 3 - v4] +
        arr[v5 * v6 + v7 - v8] +
        arr[v9 + v10 * v11 - v12] +
        arr[v13 * v14 + v15 - v16];
    
    /* Mix different data types to force mode conversions */
    char char_var = v1 & 0xFF;
    short short_var = v2 & 0xFFFF;
    int int_var = v3;
    long long_var = v4;
    
    /* Operations requiring mode conversions */
    long mixed_ops = 
        (long)char_var * int_var +
        (long)short_var / (char_var + 1) -
        long_var * (int_var - short_var);
    
    /* Use volatile in condition to prevent dead code elimination */
    volatile int condition = (complex_expr > 1000);
    if (condition) {
        /* Force function call with many parameters - uses calling convention registers */
        long func_result = helper_func(
            v1, v2, v3, v4, v5, v6
        );
        
        /* More complex expressions using pinned register variables */
        r12_var = r12_var + r13_var - r14_var * r15_var;
        r13_var = r12_var / (r14_var + 1) + r15_var - array_access;
        
        /* Another inline asm with specific constraints */
        asm volatile (
            "imulq %[val1], %[val2]\n\t"
            "addq %%r12, %[val2]\n\t"
            : [val2] "+r" (r13_var)
            : [val1] "r" (r12_var)
            : "cc"
        );
    }
    
    /* Final computation using all variables to prevent optimization */
    long final_result = 
        complex_expr + array_access + mixed_ops + 
        r12_var + r13_var + r14_var + r15_var +
        v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
        v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %ld\n", final_result % 1000);
    
    return (int)(final_result % 256);
}
