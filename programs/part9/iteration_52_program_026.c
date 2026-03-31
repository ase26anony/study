/* reload_trigger.c - Program to trigger GCC's reload pass initialization */
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
    register long r1 asm ("r12") = rand() % 100;
    register long r2 asm ("r13") = rand() % 100;
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
    
    /* Force mismatched modes */
    char c1 = rand() % 100;
    short s1 = rand() % 100;
    int i1 = rand() % 100;
    double d1 = (double)(rand() % 100);
    
    /* Complex array addressing to force address reloads */
    long arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* Complex expression with many live variables */
    long result = v1 + v2 * v3 - v4 / (v5 + 1) + v6 - v7 * v8 + v9 / (v10 + 1);
    result += v11 - v12 * v13 + v14 / (v15 + 1) - v16 + v17 * v18 - v19 / (v20 + 1);
    
    /* Mix different types to force mode conversions */
    result += (long)c1 + (long)s1 + (long)i1 + (long)d1;
    
    /* Complex array indexing with multiple variables */
    long idx = arr[v1 + v2 * v3 - v4 + r1];
    result += idx;
    
    /* Inline assembly with fixed register constraints and clobbers */
    asm volatile (
        "movq %1, %%rax\n\t"
        "addq %2, %%rax\n\t"
        "movq %%rax, %0\n\t"
        : "=r" (result)
        : "r" (result), "r" (r2)
        : "%rax", "%rbx", "%rcx", "%rdx", "memory"
    );
    
    /* More inline assembly with explicit constraints */
    long temp1, temp2;
    asm volatile (
        "movq %2, %%r10\n\t"
        "imulq %3, %%r10\n\t"
        "movq %%r10, %0\n\t"
        "movq %4, %%r11\n\t"
        "addq %%r11, %1\n\t"
        : "=r" (temp1), "=r" (temp2)
        : "r" (v1), "r" (v2), "r" (v3)
        : "%r10", "%r11", "%rax", "memory"
    );
    
    result += temp1 + temp2;
    
    /* Force address computation reloads with struct */
    struct nested {
        long a;
        struct {
            long b;
            long c;
        } inner;
        long d;
    } s = {0};
    
    long* ptr = &s.inner.b;
    *ptr = result;
    
    /* Complex condition to prevent optimization */
    volatile int condition = (result > 1000);
    if (condition) {
        /* Force function call with many parameters */
        result = helper_func(v1, v2, v3, v4, v5, v6);
        result += helper_func(v7, v8, v9, v10, v11, v12);
        result += helper_func(v13, v14, v15, v16, v17, v18);
    }
    
    /* Use all variables in final computation to prevent dead code elimination */
    result = result + r1 + r2 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                    v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                    c1 + s1 + i1 + (long)d1 + arr[0] + *ptr;
    
    printf("Result: %ld\n", result);
    return (int)(result % 256);
}
