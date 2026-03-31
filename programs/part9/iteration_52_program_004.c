/* reload_trigger.c
 * Designed to trigger GCC's reload pass initialization code
 * Compile with: gcc -O2 -fomit-frame-pointer -march=x86-64 reload_trigger.c -o reload_trigger
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Helper function that forces register-based parameter passing */
__attribute__((noinline))
static long helper_func(long a, long b, long c, long d, long e, long f) {
    volatile long result = a + b * c - d / (e + 1) + f;
    return result;
}

/* Another helper to create more register pressure */
__attribute__((noinline))
static int complex_calc(int a, int b, int c, int d, int e, int f, int g, int h) {
    return ((a * b) + (c * d) - (e * f) + (g * h)) & 0xFF;
}

int main(void) {
    /* Create register pressure with many live variables */
    volatile int v1 = 1;
    register int v2 asm ("r12") = 2;  /* Pin to specific register */
    int v3 = 3;
    long v4 = 4;
    volatile long v5 = 5;
    int v6 = 6;
    long v7 = 7;
    int v8 = 8;
    long v9 = 9;
    int v10 = 10;
    long v11 = 11;
    int v12 = 12;
    long v13 = 13;
    int v14 = 14;
    long v15 = 15;
    int v16 = 16;
    long v17 = 17;
    int v18 = 18;
    long v19 = 19;
    int v20 = 20;
    
    /* Additional variables for more pressure */
    int v21 = 21, v22 = 22, v23 = 23, v24 = 24, v25 = 25;
    long v26 = 26, v27 = 27, v28 = 28, v29 = 29, v30 = 30;
    
    /* Force mode mismatches */
    char c1 = 'A';
    short s1 = 100;
    float f1 = 3.14f;
    double d1 = 2.71828;
    
    /* Complex array addressing to force address reloads */
    int arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 2;
    }
    
    /* Inline assembly with fixed register constraints */
    /* This forces specific register allocation and clobbers */
    asm volatile (
        /* Move v2 to eax, clobbering rax */
        "movl %[v2], %%eax\n\t"
        /* Perform operation using fixed registers */
        "addl %[v3], %%eax\n\t"
        "movl %%eax, %[v1]\n\t"
        /* Clobber multiple registers */
        : [v1] "+m" (v1)
        : [v2] "r" (v2), [v3] "r" (v3)
        : "rax", "rbx", "rcx", "rdx", "memory"
    );
    
    /* Another asm with different constraints */
    long temp;
    asm volatile (
        "movq %1, %%rax\n\t"
        "imulq %2, %%rax\n\t"
        "movq %%rax, %0\n\t"
        : "=r" (temp)
        : "r" (v4), "r" (v5)
        : "rax", "cc"
    );
    
    /* Complex expression using many variables - creates register pressure */
    v6 = ((v7 + v8) * (v9 - v10)) / (v11 + 1);
    v12 = v13 * v14 - v15 / (v16 + 1) + v17 % (v18 + 1);
    
    /* More complex chain */
    int result1 = v1 + v2 * v3 - v4 / v5 + v6 - v7 * v8 + v9 / v10;
    int result2 = v11 + v12 * v13 - v14 / v15 + v16 - v17 * v18 + v19 / v20;
    int result3 = v21 + v22 * v23 - v24 / v25 + v26 - v27 * v28 + v29 / v30;
    
    /* Mix types to force mode conversions */
    v1 = c1 + s1;  /* char + short -> int promotion */
    v2 = (int)f1 + (int)d1;  /* float/double to int */
    
    /* Complex array indexing - forces address reloads */
    int idx = v1 + v2 * v3 - v4;
    volatile int array_val = arr[idx % 100] + arr[(idx + v5) % 100] * arr[(idx + v6) % 100];
    
    /* Nested struct with address taking */
    struct inner {
        int a;
        long b;
        char c[10];
    };
    
    struct outer {
        struct inner in;
        int x;
        long y;
    };
    
    struct outer outer_var = {{1, 2, "test"}, 3, 4};
    int* ptr = &outer_var.in.a;  /* Taking address */
    *ptr = v1 + v2;
    
    /* Force function calls with many parameters - uses calling convention registers */
    long func_result = helper_func(v4, v5, v6, v7, v8, v9);
    
    /* More register pressure before second call */
    int temp_calc = complex_calc(v10, v11, v12, v13, v14, v15, v16, v17);
    
    /* Use volatile to prevent optimization */
    volatile int final_check = 0;
    if (array_val > 1000 || func_result > 50 || temp_calc < 100) {
        final_check = 1;
    }
    
    /* Final complex expression using most variables */
    long final_result = 
        v1 + v2 * v3 - v4 / (v5 + 1) + 
        v6 - v7 * v8 + v9 / (v10 + 1) -
        v11 + v12 * v13 - v14 / (v15 + 1) +
        v16 - v17 * v18 + v19 / (v20 + 1) +
        v21 - v22 * v23 + v24 / (v25 + 1) +
        v26 - v27 * v28 + v29 / (v30 + 1) +
        func_result + temp_calc + array_val;
    
    printf("Final result: %ld (check: %d)\n", final_result, final_check);
    
    return (final_result > 0) ? 0 : 1;
}
