/* reload_trigger.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Helper function that forces register-based parameter passing */
__attribute__((noinline))
static long helper_func(long a, long b, long c, long d, long e, long f, long g, long h) {
    volatile long result = 0;
    /* Complex computation to prevent optimization */
    result = a * b + c * d - e * f + g * h;
    result = result ^ (a + b) ^ (c + d) ^ (e + f) ^ (g + h);
    return result;
}

/* Another helper with mismatched types */
__attribute__((noinline))
static double mixed_helper(int a, float b, double c, long d) {
    volatile double result = 0.0;
    /* Mixed mode operations */
    result = (double)a + (double)b * c - (double)d;
    return result;
}

int main(void) {
    /* Create register pressure with many live variables */
    register long r1 asm ("r12") = 1;
    register long r2 asm ("r13") = 2;
    volatile long v1 = 3, v2 = 4, v3 = 5, v4 = 6, v5 = 7;
    volatile long v6 = 8, v7 = 9, v8 = 10, v9 = 11, v10 = 12;
    volatile long v11 = 13, v12 = 14, v13 = 15, v14 = 16, v15 = 17;
    volatile long v16 = 18, v17 = 19, v18 = 20, v19 = 21, v20 = 22;
    
    /* Additional non-volatile variables */
    long nv1 = 23, nv2 = 24, nv3 = 25, nv4 = 26, nv5 = 27;
    long nv6 = 28, nv7 = 29, nv8 = 30, nv9 = 31, nv10 = 32;
    
    /* Mixed types for mode mismatches */
    char c1 = 'A', c2 = 'B', c3 = 'C';
    short s1 = 100, s2 = 200, s3 = 300;
    int i1 = 1000, i2 = 2000, i3 = 3000;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f;
    double d1 = 10.1, d2 = 20.2, d3 = 30.3;
    
    /* Complex array indexing for address reloads */
    long arr[100];
    for (int idx = 0; idx < 100; idx++) {
        arr[idx] = idx * 2;
    }
    
    /* Inline assembly with fixed register constraints */
    long asm_result;
    asm volatile (
        /* Clobber multiple registers to force reloads */
        "movq %[in1], %%rax\n\t"
        "addq %[in2], %%rax\n\t"
        "movq %%rax, %[out]\n\t"
        : [out] "=r" (asm_result)
        : [in1] "r" (v1), [in2] "r" (v2)
        : "rax", "rbx", "rcx", "rdx", "memory"
    );
    
    /* Another asm with different constraints */
    long asm_result2;
    asm volatile (
        "imulq %[in1], %[in2]\n\t"
        "movq %[in2], %[out]\n\t"
        : [out] "=r" (asm_result2)
        : [in1] "r" (v3), [in2] "r" (v4)
        : "cc"
    );
    
    /* Complex expression using many variables - creates register pressure */
    volatile long complex_expr = 0;
    complex_expr = v1 + v2 * v3 - v4 / (v5 + 1) + v6 % (v7 + 1);
    complex_expr += v8 ^ v9 | v10 & v11;
    complex_expr += (v12 << 2) | (v13 >> 3);
    complex_expr += nv1 * nv2 - nv3 * nv4 + nv5 * nv6 - nv7 * nv8;
    
    /* Mixed mode operations forcing conversions */
    volatile double mixed_result = 0.0;
    mixed_result = (double)c1 + (double)s1 * f1 - (double)i1 / d1;
    mixed_result += (float)v1 * d2 + (long)f2 - (int)d3;
    
    /* Complex array addressing - may need address reloads */
    volatile long array_access = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 5; j++) {
            /* Complex index expression */
            array_access += arr[i * 7 + j * 3 + v1 % 10];
        }
    }
    
    /* Nested structure with address taking */
    struct inner {
        long a;
        double b;
        char c[10];
    };
    
    struct outer {
        struct inner inner1;
        struct inner inner2;
        long extra;
    };
    
    struct outer outer_var;
    outer_var.inner1.a = v1;
    outer_var.inner1.b = d1;
    outer_var.inner2.a = v2;
    outer_var.inner2.b = d2;
    
    /* Taking address of nested member - may need reloads */
    long* ptr1 = &outer_var.inner1.a;
    double* ptr2 = &outer_var.inner1.b;
    char* ptr3 = &outer_var.inner2.c[3];
    
    /* Use the pointers in computations */
    *ptr1 = v3 + v4;
    *ptr2 = f1 * f2;
    *ptr3 = c1 + 1;
    
    /* Force function calls with many parameters */
    volatile long func_result = 0;
    func_result = helper_func(v1, v2, v3, v4, v5, v6, v7, v8);
    func_result += helper_func(v9, v10, v11, v12, v13, v14, v15, v16);
    
    /* Mixed type function call */
    volatile double mixed_func_result = 0.0;
    mixed_func_result = mixed_helper(i1, f1, d1, v1);
    mixed_func_result += mixed_helper(i2, f2, d2, v2);
    
    /* Final complex computation using all variables */
    volatile long final_result = 0;
    final_result = asm_result + asm_result2 + complex_expr + array_access;
    final_result += func_result + (long)mixed_result + (long)mixed_func_result;
    final_result += r1 + r2;  /* Use register variables */
    
    /* Use all local variables in final output to prevent optimization */
    final_result += nv9 + nv10 + i3 + s3 + c3;
    
    /* Conditional based on volatile to prevent dead code elimination */
    if (v1 > 0) {
        printf("Final result: %ld\n", final_result);
    } else {
        printf("Alternative: %f\n", mixed_result);
    }
    
    return (int)(final_result % 100);
}
