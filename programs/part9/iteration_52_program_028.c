/* reload_test.c - Program to trigger GCC's reload pass initialization */
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
    volatile long v2 = rand() % 100 + 1;  /* +1 to avoid division by zero */
    volatile long v3 = rand() % 100;
    volatile long v4 = rand() % 100;
    volatile long v5 = rand() % 100 + 1;
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
    
    /* Use explicit register variables to pin values to specific registers */
    /* This creates conflicts with other operands */
    register long r12_var asm ("r12") = v1 + v2;
    register long r13_var asm ("r13") = v3 * v4;
    register long r14_var asm ("r14") = v5 / v6;
    register long r15_var asm ("r15") = v7 - v8;
    
    /* Complex expression using many variables - creates register pressure */
    long complex_result = 
        v1 + v2 * v3 - v4 / (v5 + 1) + 
        v6 * v7 - v8 + v9 / (v10 + 1) +
        v11 - v12 * v13 + v14 / (v15 + 1) -
        v16 + v17 * v18 - v19 / (v20 + 1);
    
    /* Inline assembly with fixed register constraints and clobbers */
    /* This forces the compiler to work around specific register usage */
    asm volatile (
        /* Dummy operation that uses specific registers */
        "movq %[in1], %%rax\n\t"
        "addq %[in2], %%rax\n\t"
        "movq %%rax, %[out]\n\t"
        : [out] "=r" (complex_result)  /* Output operand */
        : [in1] "r" (r12_var),         /* Input operand 1 */
          [in2] "r" (r13_var)          /* Input operand 2 */
        : "rax", "rbx", "rcx", "rdx", "memory"  /* Clobber specific registers */
    );
    
    /* More complex arithmetic with mismatched operations */
    /* Mixing different operations to create mode/class mismatches */
    double d1 = (double)v1;
    float f1 = (float)v2;
    int i1 = (int)v3;
    char c1 = (char)v4;
    
    /* Operations that may require mode conversions */
    double mixed_result = d1 + f1 + i1 + c1;
    
    /* Force address reloads with complex memory addressing */
    long arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * i;
    }
    
    /* Complex array indexing with multiple variables */
    long array_access = 
        arr[v1 + v2 * 3] + 
        arr[v3 * 2 + v4] - 
        arr[v5 / 2 + v6] * 
        arr[v7 - v8 + 10];
    
    /* Another inline asm with different constraints */
    long asm_result;
    asm volatile (
        "imulq %[a], %[b]\n\t"
        "addq %[c], %[b]\n\t"
        : [b] "+r" (asm_result)
        : [a] "r" (v9),
          [c] "r" (v10)
        : "cc"  /* Clobber condition codes */
    );
    
    /* Use volatile variable in condition to prevent elimination */
    volatile int condition = (complex_result > 1000);
    if (condition) {
        /* Force function call with many arguments - uses register passing */
        long func_result = helper_func(
            v11, v12, v13, v14, v15, v16
        );
        
        /* More arithmetic to keep variables live */
        array_access += func_result * 2;
    }
    
    /* Final computation using all variables to prevent dead code elimination */
    long final_result = 
        complex_result + mixed_result + array_access + asm_result +
        r12_var + r13_var + r14_var + r15_var +
        v17 + v18 + v19 + v20;
    
    /* Use the result to prevent optimization */
    printf("Result: %ld\n", final_result);
    
    return (final_result > 0) ? 0 : 1;
}
