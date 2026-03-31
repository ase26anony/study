/* reload_test.c - Trigger GCC reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Helper function that forces register-based parameter passing */
__attribute__((noinline))
static long helper_func(long a, long b, long c, long d, long e, long f) {
    return a + b * c - d / (e + 1) + f;
}

/* Another helper to create more register pressure */
__attribute__((noinline))
static int complex_calc(int a, int b, int c, int d, int e, int f,
                        int g, int h, int i, int j) {
    return ((a * b) + (c / d) - (e % f) + (g << 2) + (h >> 3) + i - j);
}

int main(void) {
    /* Create register pressure with many live variables */
    volatile int trigger = 1;  /* Prevent optimization */
    register long r1 asm ("r12") = 100;  /* Pin to specific register */
    register long r2 asm ("r13") = 200;
    
    /* Many local variables to exhaust registers */
    long v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6, v7 = 7, v8 = 8;
    long v9 = 9, v10 = 10, v11 = 11, v12 = 12, v13 = 13, v14 = 14;
    long v15 = 15, v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    int i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5, i6 = 6, i7 = 7, i8 = 8;
    int i9 = 9, i10 = 10, i11 = 11, i12 = 12, i13 = 13, i14 = 14;
    
    /* Complex array indexing to force address reloads */
    long arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 2;
    }
    
    /* Inline assembly with fixed register constraints */
    /* This forces specific register allocation and clobbers */
    asm volatile (
        "movq %[in], %%rax\n\t"          /* Force use of rax */
        "addq %%r12, %%rax\n\t"          /* Use pinned register r12 */
        "movq %%rax, %[out]\n\t"
        : [out] "=r" (v1)                /* Output constraint */
        : [in] "r" (v2),                 /* Input constraint */
          "0" (v1)                       /* Matching constraint */
        : "rax", "rbx", "rcx", "rdx"     /* Clobber specific registers */
    );
    
    /* Another asm with mismatched modes */
    short s1 = 100;
    int result;
    asm volatile (
        "movswl %w1, %0\n\t"             /* Sign extend short to int */
        "addl $100, %0\n\t"
        : "=r" (result)
        : "r" (s1)
        : "cc"
    );
    
    /* Complex expression using many variables - creates register pressure */
    if (trigger) {
        v3 = v4 + v5 * v6 - v7 / (v8 + 1) + v9 - v10;
        v11 = v12 * v13 + v14 / v15 - v16 % v17 + v18;
        
        /* Very long chain of operations */
        v19 = v1 + v2 - v3 * v4 / v5 + v6 - v7 + v8 * v9 - v10;
        v20 = v11 + v12 - v13 * v14 / v15 + v16 - v17 + v18 * v19 - v20;
        
        /* Mix different types to force mode conversions */
        double d1 = v1 * 1.5;
        float f1 = v2 * 2.5f;
        d1 = d1 + f1;  /* Mixed precision */
        
        /* Complex addressing mode */
        long idx = v1 + v2 * v3 - v4;
        if (idx >= 0 && idx < 100) {
            v1 = arr[idx] + arr[v5 + v6 * 2 - v7];
        }
    }
    
    /* Force function calls with many parameters */
    long func_result = helper_func(v1, v2, v3, v4, v5, v6);
    int int_result = complex_calc(i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
    
    /* Use volatile to prevent dead code elimination */
    volatile long final_check = func_result + int_result + v20 + result;
    
    /* More inline assembly with explicit constraints */
    long out1, out2;
    asm volatile (
        "movq %2, %%r10\n\t"
        "movq %3, %%r11\n\t"
        "addq %%r10, %%r11\n\t"
        "movq %%r11, %0\n\t"
        "imulq %4, %%r11\n\t"
        "movq %%r11, %1\n\t"
        : "=r" (out1), "=r" (out2)
        : "r" (v1), "r" (v2), "r" (v3)
        : "r10", "r11", "cc"
    );
    
    /* Struct with nested addressing */
    struct nested {
        int a;
        struct {
            int x;
            int y;
            long arr[5];
        } inner;
        int b;
    } s;
    
    s.a = 10;
    s.inner.x = 20;
    s.inner.y = 30;
    s.b = 40;
    
    /* Take address of nested member - can force address reloads */
    int *ptr = &s.inner.x;
    *ptr = final_check % 100;
    
    /* Final computation using everything */
    long final_result = 
        v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
        v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
        func_result + int_result + out1 + out2 + *ptr + r1 + r2;
    
    printf("Result: %ld\n", final_result);
    return (final_result > 0) ? 0 : 1;
}
