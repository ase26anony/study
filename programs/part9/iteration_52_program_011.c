/* reload_trigger.c - Program to trigger GCC reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Helper function that forces register-based parameter passing */
__attribute__((noinline))
static long helper_func(long a, long b, long c, long d, 
                        long e, long f, long g, long h) {
    volatile long barrier = 0;
    (void)barrier;
    return a + b * c - d / (e + 1) + f - g * h;
}

int main(void) {
    /* Seed random number generator */
    srand(time(NULL));
    
    /* Create register pressure with many live variables */
    volatile long v1 = rand() % 100 + 1;
    register long v2 asm ("r12") = rand() % 100 + 1;
    register long v3 asm ("r13") = rand() % 100 + 1;
    long v4 = rand() % 100 + 1;
    long v5 = rand() % 100 + 1;
    long v6 = rand() % 100 + 1;
    long v7 = rand() % 100 + 1;
    long v8 = rand() % 100 + 1;
    long v9 = rand() % 100 + 1;
    long v10 = rand() % 100 + 1;
    long v11 = rand() % 100 + 1;
    long v12 = rand() % 100 + 1;
    long v13 = rand() % 100 + 1;
    long v14 = rand() % 100 + 1;
    long v15 = rand() % 100 + 1;
    long v16 = rand() % 100 + 1;
    long v17 = rand() % 100 + 1;
    long v18 = rand() % 100 + 1;
    long v19 = rand() % 100 + 1;
    long v20 = rand() % 100 + 1;
    
    /* Inline assembly with fixed register constraints */
    /* This forces specific register allocation and creates conflicts */
    asm volatile (
        /* Clobber multiple registers to force reloads */
        "movq %[in1], %%rax\n\t"
        "addq %[in2], %%rax\n\t"
        "movq %%rax, %[out]\n\t"
        : [out] "=r" (v1)          /* Output operand */
        : [in1] "r" (v2),          /* Input operand 1 */
          [in2] "r" (v3)           /* Input operand 2 */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "cc", "memory"
    );
    
    /* Complex expression creating register pressure */
    /* Mix operations with different modes/types */
    v4 = (v5 + v6) * (v7 - v8) / (v9 | 1);
    v10 = v11 * v12 + v13 - v14 / (v15 + 1);
    
    /* Force mode mismatches */
    {
        char c1 = (char)v16;
        short s1 = (short)v17;
        int i1 = (int)v18;
        /* Mix different integer sizes */
        v19 = c1 + s1 * i1 - (v20 & 0xFF);
    }
    
    /* Complex array indexing to force address reloads */
    {
        long arr[100];
        for (int i = 0; i < 100; i++) {
            arr[i] = i * i;
        }
        /* Complex addressing mode */
        volatile long idx = v1 + v2 * v3 - v4 / (v5 + 1);
        long result = arr[(idx + v6 * v7 - v8) % 100];
        (void)result;
    }
    
    /* Use volatile to prevent dead code elimination */
    volatile int condition = (v1 > v2) && (v3 < v4) || (v5 == v6);
    
    if (condition) {
        /* Force function call with many parameters */
        long func_result = helper_func(v1, v2, v3, v4, v5, v6, v7, v8);
        v9 = func_result + v10 - v11 * v12;
    } else {
        /* Alternative path with different register usage */
        v9 = v13 * v14 - v15 / (v16 + 1) + v17 - v18 * v19;
    }
    
    /* More inline assembly with explicit constraints */
    {
        long temp1, temp2, temp3;
        asm volatile (
            "movq %2, %%r10\n\t"
            "imulq %3, %%r10\n\t"
            "addq %4, %%r10\n\t"
            "movq %%r10, %0\n\t"
            "movq %5, %%r11\n\t"
            "subq %%r10, %%r11\n\t"
            "movq %%r11, %1\n\t"
            : "=r" (temp1), "=r" (temp2)
            : "r" (v1), "r" (v2), "r" (v3), "r" (v4)
            : "r10", "r11", "cc"
        );
        v20 = temp1 + temp2;
    }
    
    /* Final computation using all variables */
    long final_result = 
        v1 + v2 - v3 * v4 / (v5 + 1) + 
        v6 - v7 * v8 + v9 / (v10 | 1) -
        v11 + v12 * v13 - v14 / (v15 + 1) +
        v16 - v17 * v18 + v19 / (v20 | 1);
    
    /* Use the result to prevent optimization */
    printf("Result: %ld\n", final_result);
    
    return (final_result > 0) ? 0 : 1;
}
