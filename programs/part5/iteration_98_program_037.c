/* test_caller_save.c - Forces GCC to insert save/restore instructions at call sites */

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining of called functions */
__attribute__((noinline)) void foo(void) {
    /* Empty function that compiler cannot inline */
    asm volatile("" : : : "memory");
}

__attribute__((noinline)) void bar(void) {
    asm volatile("" : : : "memory");
}

__attribute__((noinline)) void baz(void) {
    asm volatile("" : : : "memory");
}

/* Helper to create register pressure */
#define DECLARE_INT_VARS(n) \
    volatile int int_var_##n##_0, int_var_##n##_1, int_var_##n##_2, int_var_##n##_3; \
    volatile int int_var_##n##_4, int_var_##n##_5, int_var_##n##_6, int_var_##n##_7; \
    volatile int int_var_##n##_8, int_var_##n##_9, int_var_##n##_10, int_var_##n##_11; \
    volatile int int_var_##n##_12, int_var_##n##_13, int_var_##n##_14, int_var_##n##_15

#define USE_INT_VARS(n) \
    int_var_##n##_0 = n * 1;  int_var_##n##_1 = n * 2;  \
    int_var_##n##_2 = n * 3;  int_var_##n##_3 = n * 4;  \
    int_var_##n##_4 = n * 5;  int_var_##n##_5 = n * 6;  \
    int_var_##n##_6 = n * 7;  int_var_##n##_7 = n * 8;  \
    int_var_##n##_8 = n * 9;  int_var_##n##_9 = n * 10; \
    int_var_##n##_10 = n * 11; int_var_##n##_11 = n * 12; \
    int_var_##n##_12 = n * 13; int_var_##n##_13 = n * 14; \
    int_var_##n##_14 = n * 15; int_var_##n##_15 = n * 16

/* Test 1: Integer register pressure with call at end of basic block */
__attribute__((noinline)) int test_integer_pressure(int x, int y) {
    DECLARE_INT_VARS(1);
    DECLARE_INT_VARS(2);
    DECLARE_INT_VARS(3);
    DECLARE_INT_VARS(4);
    
    /* Create complex control flow with call at block end */
    if (x > y) {
        /* This creates a basic block ending with the call */
        USE_INT_VARS(1);
        USE_INT_VARS(2);
        
        /* Clobber many integer registers */
        asm volatile("" : : : 
            "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
            "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
            "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
            "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
            "xmm12", "xmm13", "xmm14", "xmm15");
        
        foo();  /* Call at end of basic block */
        
        /* Use variables after call */
        return int_var_1_0 + int_var_2_1 + int_var_3_2 + int_var_4_3;
    } else {
        /* Alternative path */
        USE_INT_VARS(3);
        USE_INT_VARS(4);
        bar();
        return int_var_3_0 + int_var_4_1;
    }
}

/* Test 2: Floating-point pressure with switch statement */
__attribute__((noinline)) double test_fp_pressure(double a, double b, int mode) {
    volatile double fp0, fp1, fp2, fp3, fp4, fp5, fp6, fp7;
    volatile double fp8, fp9, fp10, fp11, fp12, fp13, fp14, fp15;
    volatile double fp16, fp17, fp18, fp19, fp20, fp21, fp22, fp23;
    
    /* Create many FP computations */
    fp0 = sin(a);
    fp1 = cos(b);
    fp2 = fp0 * fp1;
    fp3 = fp1 / fp0;
    fp4 = fp2 + fp3;
    fp5 = fp3 - fp2;
    fp6 = fp4 * fp5;
    fp7 = fp5 / fp4;
    fp8 = fp6 + fp7;
    fp9 = fp7 - fp6;
    fp10 = fp8 * fp9;
    fp11 = fp9 / fp8;
    fp12 = fp10 + fp11;
    fp13 = fp11 - fp10;
    fp14 = fp12 * fp13;
    fp15 = fp13 / fp12;
    fp16 = fp14 + fp15;
    fp17 = fp15 - fp14;
    fp18 = fp16 * fp17;
    fp19 = fp17 / fp16;
    fp20 = fp18 + fp19;
    fp21 = fp19 - fp18;
    fp22 = fp20 * fp21;
    fp23 = fp21 / fp20;
    
    /* Switch creates multiple basic blocks */
    switch (mode) {
        case 0:
            /* Call at end of this case's basic block */
            foo();
            return fp0 + fp1 + fp2;
        case 1:
            bar();
            return fp3 + fp4 + fp5;
        case 2:
            baz();
            return fp6 + fp7 + fp8;
        case 3:
            /* More register pressure before call */
            asm volatile("" : : : 
                "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                "xmm12", "xmm13", "xmm14", "xmm15");
            foo();  /* Call at block end */
            return fp9 + fp10 + fp11;
        default:
            return fp12 + fp13 + fp14;
    }
}

/* Test 3: Vector/SIMD pressure with loop unrolling */
__attribute__((noinline)) __m128 test_vector_pressure(__m128 a, __m128 b) {
    __m128 v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    __m128 v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    
    /* Create many vector operations */
    v0 = _mm_add_ps(a, b);
    v1 = _mm_sub_ps(a, b);
    v2 = _mm_mul_ps(v0, v1);
    v3 = _mm_div_ps(v0, v1);
    v4 = _mm_add_ps(v2, v3);
    v5 = _mm_sub_ps(v2, v3);
    v6 = _mm_mul_ps(v4, v5);
    v7 = _mm_div_ps(v4, v5);
    v8 = _mm_add_ps(v6, v7);
    v9 = _mm_sub_ps(v6, v7);
    v10 = _mm_mul_ps(v8, v9);
    v11 = _mm_div_ps(v8, v9);
    v12 = _mm_add_ps(v10, v11);
    v13 = _mm_sub_ps(v10, v11);
    v14 = _mm_mul_ps(v12, v13);
    v15 = _mm_div_ps(v12, v13);
    v16 = _mm_add_ps(v14, v15);
    v17 = _mm_sub_ps(v14, v15);
    v18 = _mm_mul_ps(v16, v17);
    v19 = _mm_div_ps(v16, v17);
    
    /* Partially unrolled loop with call at end */
    for (int i = 0; i < 2; i++) {
        if (i == 0) {
            /* This creates a basic block ending with call */
            asm volatile("" : : : 
                "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5",
                "ymm6", "ymm7", "ymm8", "ymm9", "ymm10", "ymm11",
                "ymm12", "ymm13", "ymm14", "ymm15");
            foo();  /* Call at block end */
        } else {
            bar();
        }
    }
    
    return _mm_add_ps(v18, v19);
}

/* Test 4: Mixed pressure with nested control flow */
__attribute__((noinline)) int test_mixed_pressure(int x, double y, __m128 z) {
    volatile int i0, i1, i2, i3, i4, i5, i6, i7;
    volatile double d0, d1, d2, d3, d4, d5, d6, d7;
    __m128 v0, v1, v2, v3;
    
    /* Initialize variables */
    i0 = x * 1; i1 = x * 2; i2 = x * 3; i3 = x * 4;
    i4 = x * 5; i5 = x * 6; i6 = x * 7; i7 = x * 8;
    
    d0 = sin(y); d1 = cos(y); d2 = d0 * d1; d3 = d1 / d0;
    d4 = d2 + d3; d5 = d2 - d3; d6 = d4 * d5; d7 = d4 / d5;
    
    v0 = _mm_add_ps(z, z);
    v1 = _mm_sub_ps(z, z);
    v2 = _mm_mul_ps(v0, v1);
    v3 = _mm_div_ps(v0, v1);
    
    /* Complex nested if-else with calls at block ends */
    if (x > 0) {
        if (y > 0.0) {
            /* Call at end of inner block */
            asm volatile("" : : : 
                "rax", "rbx", "rcx", "rdx",
                "xmm0", "xmm1", "xmm2", "xmm3",
                "ymm0", "ymm1", "ymm2", "ymm3");
            foo();
            return i0 + (int)d0;
        } else {
            bar();
            return i1 + (int)d1;
        }
    } else {
        baz();
        return i2 + (int)d2;
    }
}

/* Main function that exercises all test cases */
int main(void) {
    int result = 0;
    double fp_result = 0.0;
    __m128 vec_result;
    float vec_store[4];
    
    /* Test 1: Integer pressure */
    result += test_integer_pressure(100, 50);
    result += test_integer_pressure(10, 20);
    
    /* Test 2: FP pressure */
    fp_result += test_fp_pressure(1.0, 2.0, 0);
    fp_result += test_fp_pressure(3.0, 4.0, 3);
    
    /* Test 3: Vector pressure */
    __m128 a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 b = _mm_set_ps(5.0f, 6.0f, 7.0f, 8.0f);
    vec_result = test_vector_pressure(a, b);
    _mm_store_ps(vec_store, vec_result);
    result += (int)vec_store[0];
    
    /* Test 4: Mixed pressure */
    result += test_mixed_pressure(42, 3.14159, a);
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d, FP: %f\n", result, fp_result);
    
    return 0;
}
