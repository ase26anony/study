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
    volatile long v1 = rand() % 100 + 1;
    register long v2 asm ("r12") = v1 + 1;  /* Pin to specific register */
    long v3 = v2 * 2;
    long v4 = v3 - 5;
    long v5 = v4 / 2;
    long v6 = v5 + 10;
    long v7 = v6 * 3;
    long v8 = v7 - 7;
    long v9 = v8 / 4;
    long v10 = v9 + 15;
    long v11 = v10 * 2;
    long v12 = v11 - 3;
    long v13 = v12 / 2;
    long v14 = v13 + 8;
    long v15 = v14 * 3;
    long v16 = v15 - 9;
    long v17 = v16 / 2;
    long v18 = v17 + 20;
    long v19 = v18 * 2;
    long v20 = v19 - 5;
    
    /* Complex array indexing to force address reloads */
    long arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * i;
    }
    
    /* Complex addressing mode - forces address computation reloads */
    volatile long array_access = arr[v1 + v2 * v3 - v4];
    
    /* Inline assembly with fixed register constraints and clobbers */
    /* This forces specific register allocation and potential conflicts */
    asm volatile (
        "movq %[input], %%rax\n\t"        /* Force use of rax */
        "addq $1, %%rax\n\t"
        "movq %%rax, %[output]\n\t"
        : [output] "=r" (v3)              /* Output operand */
        : [input] "r" (v2)                /* Input operand */
        : "rax", "rbx", "rcx", "rdx"      /* Clobber specific registers */
    );
    
    /* Another inline asm with mismatched constraints */
    long temp;
    asm volatile (
        "movl %1, %0\n\t"
        : "=r" (temp)                     /* General register output */
        : "m" (v4)                        /* Memory input - mismatch */
        : "cc"
    );
    
    /* Complex expression using most variables - maximizes live ranges */
    v1 = v2 + v3 * v4 - v5 / (v6 + 1) + v7 - v8 * v9 + v10 / (v11 + 1);
    v12 = v13 + v14 * v15 - v16 / (v17 + 1) + v18 - v19 * v20 + v1;
    
    /* Mode mixing - char in int operation */
    char c1 = 65;
    int i1 = c1 * v2;                     /* Mode conversion needed */
    
    /* Float/double mixing */
    float f1 = 3.14f;
    double d1 = f1 * v3;                  /* Mode conversion */
    
    /* Force function call with many parameters - uses call-clobbered regs */
    long func_result = helper_func(v1, v2, v3, v4, v5, v6);
    
    /* More complex operations to prevent optimization */
    for (int i = 0; i < 10; i++) {
        v1 = v1 * v2 + v3 - v4;
        v2 = v2 / (v5 + 1) * v6;
        v3 = v3 + v7 * v8 - v9;
        
        /* Nested struct with address taken */
        struct nested {
            long a;
            struct {
                long x;
                long y;
            } inner;
            long b;
        } s;
        
        s.a = v1;
        s.inner.x = v2;
        s.inner.y = v3;
        s.b = v4;
        
        /* Taking address of nested member forces complex addressing */
        long *ptr = &s.inner.x;
        *ptr += i;
    }
    
    /* Final volatile use to prevent dead code elimination */
    volatile long final_result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                                v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                                func_result + array_access + temp + i1 + (long)d1;
    
    printf("Result: %ld\n", final_result);
    return (int)(final_result % 100);
}
