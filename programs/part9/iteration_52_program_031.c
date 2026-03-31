/* reload_coverage.c
 * Designed to trigger GCC's reload pass initialization logic
 * Compile with: gcc -O3 -fomit-frame-pointer -march=x86-64 reload_coverage.c -o reload_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Helper function that forces register-based parameter passing */
__attribute__((noinline))
static long helper_func(long a, long b, long c, long d, long e, long f) {
    volatile long barrier = a + b + c + d + e + f;
    return barrier * 2;
}

/* Another helper to create more register pressure */
__attribute__((noinline))
static void complex_operation(volatile long* results, int count) {
    for (int i = 0; i < count; i++) {
        results[i] = results[i] * 3 + i;
    }
}

int main(void) {
    /* Seed random for varied initialization */
    srand(time(NULL));
    
    /* Create massive register pressure with many live variables */
    /* Using 'volatile' on some to prevent optimization */
    volatile long v1 = rand() % 100 + 1;
    long v2 = rand() % 100 + 2;
    volatile long v3 = rand() % 100 + 3;
    long v4 = rand() % 100 + 4;
    volatile long v5 = rand() % 100 + 5;
    long v6 = rand() % 100 + 6;
    volatile long v7 = rand() % 100 + 7;
    long v8 = rand() % 100 + 8;
    volatile long v9 = rand() % 100 + 9;
    long v10 = rand() % 100 + 10;
    volatile long v11 = rand() % 100 + 11;
    long v12 = rand() % 100 + 12;
    volatile long v13 = rand() % 100 + 13;
    long v14 = rand() % 100 + 14;
    volatile long v15 = rand() % 100 + 15;
    long v16 = rand() % 100 + 16;
    volatile long v17 = rand() % 100 + 17;
    long v18 = rand() % 100 + 18;
    volatile long v19 = rand() % 100 + 19;
    long v20 = rand() % 100 + 20;
    
    /* Additional variables for more pressure */
    long v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* Force specific registers with inline asm and clobber many registers */
    /* This creates conflicts requiring reloads */
    asm volatile (
        /* Perform some dummy operations */
        "movq %[in1], %%rax\n\t"
        "addq %[in2], %%rax\n\t"
        "movq %%rax, %[out1]\n\t"
        
        /* More operations to use/clobber registers */
        "movq %[in3], %%rbx\n\t"
        "imulq %[in4], %%rbx\n\t"
        "movq %%rbx, %[out2]\n\t"
        
        /* Use more registers */
        "movq %[in5], %%rcx\n\t"
        "movq %[in6], %%rdx\n\t"
        "addq %%rcx, %%rdx\n\t"
        "movq %%rdx, %[out3]\n\t"
        
        /* Clobber list forces save/restore or reloads */
        : [out1] "=r" (v21), [out2] "=r" (v22), [out3] "=r" (v23)
        : [in1] "r" (v1), [in2] "r" (v2), [in3] "r" (v3),
          [in4] "r" (v4), [in5] "r" (v5), [in6] "r" (v6)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "cc", "memory"
    );
    
    /* Complex expression using most variables - creates long live ranges */
    v24 = v7 + v8 * v9 - v10 / v11 + v12 % v13;
    v25 = v14 * v15 + v16 - v17 / v18 + v19 % v20;
    v26 = v21 + v22 - v23 * v24 / v25;
    
    /* More complex chain */
    v27 = (v1 * v2) + (v3 / v4) - (v5 % v6) + (v7 << 2) - (v8 >> 1);
    v28 = (v9 | v10) & (v11 ^ v12) + (v13 * v14) - (v15 / v16);
    v29 = v27 + v28 * v26 - v25 / v24 + v23 % v22;
    
    /* Force mode mismatches - mixing different sized operations */
    {
        char c1 = v1 & 0xFF;
        short s1 = v2 & 0xFFFF;
        int i1 = v3;
        long l1 = v4;
        
        /* Operations requiring promotions */
        v30 = c1 + s1 * i1 - l1 / v5;
        
        /* Floating point to force different register classes */
        double d1 = (double)v6;
        float f1 = (float)v7;
        volatile double d2 = d1 * f1;  /* Mixing float/double */
    }
    
    /* Complex array indexing - forces address reloads */
    long arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * i;
    }
    
    /* Complex addressing mode */
    volatile long arr_val = arr[v8 + v9 * v10 - v11 / v12];
    
    /* Call helper function - forces parameter passing in registers */
    v30 = helper_func(v1, v2, v3, v4, v5, v6);
    
    /* Another call with different parameters */
    v30 += helper_func(v7, v8, v9, v10, v11, v12);
    
    /* Use explicit register variables to pin values */
    register long r12_var asm ("r12") = v13;
    register long r13_var asm ("r13") = v14;
    
    /* Operations with pinned registers create conflicts */
    asm volatile (
        "addq %1, %0\n\t"
        : "+r" (r12_var)
        : "r" (r13_var)
        : "cc"
    );
    
    /* More inline asm with fixed constraints */
    long out1, out2;
    asm volatile (
        "movq %2, %%rax\n\t"
        "addq %3, %%rax\n\t"
        "movq %%rax, %0\n\t"
        "movq %4, %%rbx\n\t"
        "subq %5, %%rbx\n\t"
        "movq %%rbx, %1\n\t"
        : "=r" (out1), "=r" (out2)
        : "r" (v15), "r" (v16), "r" (v17), "r" (v18)
        : "rax", "rbx", "cc"
    );
    
    /* Create a volatile condition to prevent dead code elimination */
    volatile int condition = (v1 + v2 + v3) > 1000;
    if (condition) {
        /* Complex operation on array */
        complex_operation(arr, 50);
    }
    
    /* Final computation using all variables to ensure they're not optimized away */
    long final_result = 
        v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
        v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
        v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30 +
        out1 + out2 + r12_var + arr_val;
    
    printf("Final result: %ld\n", final_result);
    
    return (final_result > 0) ? 0 : 1;
}
