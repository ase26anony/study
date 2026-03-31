/* reload_test.c - Program to trigger GCC reload pass initialization */
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
static int complex_calc(int x1, int x2, int x3, int x4, int x5) {
    return (x1 * x2) + (x3 / (x4 ? x4 : 1)) - (x5 << 2);
}

int main(void) {
    /* Create register pressure with many live variables */
    register long r1 asm ("r12") = 1;
    register long r2 asm ("r13") = 2;
    volatile long v1 = 100;
    volatile long v2 = 200;
    volatile long v3 = 300;
    volatile long v4 = 400;
    volatile long v5 = 500;
    volatile long v6 = 600;
    volatile long v7 = 700;
    volatile long v8 = 800;
    volatile long v9 = 900;
    volatile long v10 = 1000;
    
    int i1 = 10, i2 = 20, i3 = 30, i4 = 40, i5 = 50;
    int i6 = 60, i7 = 70, i8 = 80, i9 = 90, i10 = 100;
    int i11 = 110, i12 = 120, i13 = 130, i14 = 140, i15 = 150;
    int i16 = 160, i17 = 170, i18 = 180, i19 = 190, i20 = 200;
    
    /* Complex array indexing to force address reloads */
    int arr[100];
    for (int idx = 0; idx < 100; idx++) {
        arr[idx] = idx * 2;
    }
    
    /* Inline assembly with fixed register constraints */
    long asm_result;
    asm volatile (
        /* Clobber multiple registers to force reloads */
        "movq %[input1], %%rax\n\t"
        "addq %[input2], %%rax\n\t"
        "movq %%rax, %[output]\n\t"
        : [output] "=r" (asm_result)
        : [input1] "r" (v1), [input2] "r" (v2)
        : "rax", "rbx", "rcx", "rdx", "memory"
    );
    
    /* More inline assembly with mismatched constraints */
    int asm_result2;
    asm volatile (
        "movl %1, %%eax\n\t"
        "imull %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (asm_result2)
        : "r" (i1), "r" (i2)
        : "%eax", "%ebx"
    );
    
    /* Complex expression using many variables - creates register pressure */
    long complex_result = 
        (v1 + v2) * (v3 - v4) / (v5 ? v5 : 1) +
        (v6 * v7) - (v8 / v9) +
        (i1 * i2) + (i3 / i4) - (i5 << 2) +
        (i6 + i7) * (i8 - i9) +
        (i10 * i11) - (i12 / i13) +
        (i14 << 3) + (i15 >> 1) +
        (i16 & i17) | (i18 ^ i19) +
        r1 * r2;
    
    /* Force mode mismatches */
    char c1 = 65;
    short s1 = 1000;
    int mode_mismatch = c1 * s1 + i1;  /* char -> int promotion */
    
    float f1 = 3.14f;
    double d1 = 2.71828;
    double float_mix = f1 * d1 + i2;   /* float -> double promotion */
    
    /* Complex array access with multiple index variables */
    int arr_index = arr[i1 + i2 * i3 - i4 / (i5 ? i5 : 1)];
    
    /* Call helper function - forces parameter passing in registers */
    long helper_result = helper_func(v1, v2, v3, v4, v5, v6);
    
    /* Another complex calculation */
    int calc_result = complex_calc(i1, i2, i3, i4, i5);
    
    /* Use volatile variable to prevent optimization */
    volatile int final_check = 0;
    if (complex_result > 1000) {
        final_check = 1;
    }
    
    /* Use all results to prevent dead code elimination */
    printf("Results: %ld %d %ld %d %d %d %lf\n", 
           complex_result, asm_result2, helper_result, 
           calc_result, arr_index, mode_mismatch, float_mix);
    
    return final_check;
}
