/* test_caller_save.c - Forces GCC to insert save/restore instructions at call sites */

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline, noipa))

/* External function declarations - cannot be inlined */
NOINLINE void foo(void);
NOINLINE void bar(int, double);
NOINLINE double compute_something(double, double);
NOINLINE int process_data(int, int, int, int, int, int, int, int);

/* Helper to create many live variables */
#define DECLARE_INT_VARS(n) \
    volatile int int_var_##n##_0, int_var_##n##_1, int_var_##n##_2, int_var_##n##_3, \
    int_var_##n##_4, int_var_##n##_5, int_var_##n##_6, int_var_##n##_7

#define DECLARE_FP_VARS(n) \
    volatile double fp_var_##n##_0, fp_var_##n##_1, fp_var_##n##_2, fp_var_##n##_3, \
    fp_var_##n##_4, fp_var_##n##_5, fp_var_##n##_6, fp_var_##n##_7

/* Test 1: Integer register pressure with call at end of basic block */
NOINLINE int test_integer_pressure(int a, int b, int c, int d) {
    /* Create many integer variables that must survive across call */
    DECLARE_INT_VARS(1);
    DECLARE_INT_VARS(2);
    DECLARE_INT_VARS(3);
    DECLARE_INT_VARS(4);
    
    /* Complex computation creating many live values */
    int_var_1_0 = a + 1;
    int_var_1_1 = int_var_1_0 * 2;
    int_var_1_2 = int_var_1_1 + b;
    int_var_1_3 = int_var_1_2 * int_var_1_0;
    int_var_1_4 = int_var_1_3 - c;
    int_var_1_5 = int_var_1_4 / (a + 2);
    int_var_1_6 = int_var_1_5 ^ d;
    int_var_1_7 = int_var_1_6 << 2;
    
    int_var_2_0 = b + 3;
    int_var_2_1 = int_var_2_0 * 4;
    int_var_2_2 = int_var_2_1 + c;
    int_var_2_3 = int_var_2_2 * int_var_2_0;
    int_var_2_4 = int_var_2_3 - d;
    int_var_2_5 = int_var_2_4 / (b + 4);
    int_var_2_6 = int_var_2_5 ^ a;
    int_var_2_7 = int_var_2_6 << 3;
    
    int_var_3_0 = c + 5;
    int_var_3_1 = int_var_3_0 * 6;
    int_var_3_2 = int_var_3_1 + d;
    int_var_3_3 = int_var_3_2 * int_var_3_0;
    int_var_3_4 = int_var_3_3 - a;
    int_var_3_5 = int_var_3_4 / (c + 6);
    int_var_3_6 = int_var_3_5 ^ b;
    int_var_3_7 = int_var_3_6 << 4;
    
    int_var_4_0 = d + 7;
    int_var_4_1 = int_var_4_0 * 8;
    int_var_4_2 = int_var_4_1 + a;
    int_var_4_3 = int_var_4_2 * int_var_4_0;
    int_var_4_4 = int_var_4_3 - b;
    int_var_4_5 = int_var_4_4 / (d + 8);
    int_var_4_6 = int_var_4_5 ^ c;
    int_var_4_7 = int_var_4_6 << 5;
    
    /* Use control flow to create basic block ending with call */
    if (a > b) {
        /* This creates a basic block ending with the call */
        
        /* Clobber many caller-saved registers with inline asm */
        asm volatile("" : : : 
            "rax", "rcx", "rdx", "rsi", "rdi", 
            "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
            "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
            "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15");
        
        /* Call at end of basic block */
        foo();
        
        /* Use all variables after call to keep them live */
        return int_var_1_0 + int_var_1_1 + int_var_1_2 + int_var_1_3 +
               int_var_1_4 + int_var_1_5 + int_var_1_6 + int_var_1_7 +
               int_var_2_0 + int_var_2_1 + int_var_2_2 + int_var_2_3 +
               int_var_2_4 + int_var_2_5 + int_var_2_6 + int_var_2_7 +
               int_var_3_0 + int_var_3_1 + int_var_3_2 + int_var_3_3 +
               int_var_3_4 + int_var_3_5 + int_var_3_6 + int_var_3_7 +
               int_var_4_0 + int_var_4_1 + int_var_4_2 + int_var_4_3 +
               int_var_4_4 + int_var_4_5 + int_var_4_6 + int_var_4_7;
    } else {
        /* Alternative path */
        return a + b + c + d;
    }
}

/* Test 2: Floating-point register pressure */
NOINLINE double test_fp_pressure(double x, double y, double z) {
    DECLARE_FP_VARS(1);
    DECLARE_FP_VARS(2);
    DECLARE_FP_VARS(3);
    
    /* Create many FP computations */
    fp_var_1_0 = sin(x);
    fp_var_1_1 = cos(x);
    fp_var_1_2 = fp_var_1_0 * fp_var_1_1;
    fp_var_1_3 = tan(x);
    fp_var_1_4 = exp(x);
    fp_var_1_5 = log(fabs(x) + 1.0);
    fp_var_1_6 = sqrt(fabs(x));
    fp_var_1_7 = fp_var_1_2 + fp_var_1_3 + fp_var_1_4;
    
    fp_var_2_0 = sin(y);
    fp_var_2_1 = cos(y);
    fp_var_2_2 = fp_var_2_0 * fp_var_2_1;
    fp_var_2_3 = tan(y);
    fp_var_2_4 = exp(y);
    fp_var_2_5 = log(fabs(y) + 1.0);
    fp_var_2_6 = sqrt(fabs(y));
    fp_var_2_7 = fp_var_2_2 + fp_var_2_3 + fp_var_2_4;
    
    fp_var_3_0 = sin(z);
    fp_var_3_1 = cos(z);
    fp_var_3_2 = fp_var_3_0 * fp_var_3_1;
    fp_var_3_3 = tan(z);
    fp_var_3_4 = exp(z);
    fp_var_3_5 = log(fabs(z) + 1.0);
    fp_var_3_6 = sqrt(fabs(z));
    fp_var_3_7 = fp_var_3_2 + fp_var_3_3 + fp_var_3_4;
    
    /* Use switch to create complex CFG */
    int choice = (int)x % 3;
    double result = 0.0;
    
    switch (choice) {
        case 0:
            /* Call at end of this case's basic block */
            bar(1, x);
            result = fp_var_1_0 + fp_var_1_1 + fp_var_1_2;
            break;
        case 1:
            bar(2, y);
            result = fp_var_2_0 + fp_var_2_1 + fp_var_2_2;
            break;
        case 2:
            /* This case ends with call at block end */
            bar(3, z);
            result = fp_var_3_0 + fp_var_3_1 + fp_var_3_2;
            break;
    }
    
    /* Use all FP variables after calls */
    return result + fp_var_1_3 + fp_var_1_4 + fp_var_1_5 + fp_var_1_6 + fp_var_1_7 +
                   fp_var_2_3 + fp_var_2_4 + fp_var_2_5 + fp_var_2_6 + fp_var_2_7 +
                   fp_var_3_3 + fp_var_3_4 + fp_var_3_5 + fp_var_3_6 + fp_var_3_7;
}

/* Test 3: Mixed register pressure with loop unrolling */
NOINLINE int test_mixed_pressure(int iterations) {
    volatile int sum = 0;
    volatile double prod = 1.0;
    
    /* Partially unrolled loop to create basic blocks ending with calls */
    for (int i = 0; i < iterations; i += 4) {
        /* Create many live values */
        int a0 = i + 1;
        int b0 = i + 2;
        int c0 = i + 3;
        int d0 = i + 4;
        double x0 = sin(i * 0.1);
        double y0 = cos(i * 0.1);
        
        int a1 = i + 5;
        int b1 = i + 6;
        int c1 = i + 7;
        int d1 = i + 8;
        double x1 = sin((i + 1) * 0.1);
        double y1 = cos((i + 1) * 0.1);
        
        /* Call in the middle of unrolled loop - may be at block end */
        if (i % 8 == 0) {
            /* This could be at end of a basic block */
            int r = process_data(a0, b0, c0, d0, a1, b1, c1, d1);
            sum += r;
        }
        
        /* More computations keeping variables live */
        prod *= x0 * y0 * x1 * y1;
        
        /* Another call that might be at block end */
        if (i % 4 == 0) {
            double temp = compute_something(x0, y0);
            prod *= temp;
        }
    }
    
    return sum + (int)prod;
}

/* Test 4: Vector register pressure (if available) */
#ifdef __SSE2__
#include <emmintrin.h>
NOINLINE __m128 test_vector_pressure(__m128 a, __m128 b, __m128 c) {
    /* Create many vector variables */
    __m128 v0 = _mm_add_ps(a, b);
    __m128 v1 = _mm_mul_ps(a, b);
    __m128 v2 = _mm_sub_ps(a, b);
    __m128 v3 = _mm_add_ps(v0, v1);
    __m128 v4 = _mm_mul_ps(v1, v2);
    __m128 v5 = _mm_sub_ps(v2, v3);
    __m128 v6 = _mm_add_ps(v3, v4);
    __m128 v7 = _mm_mul_ps(v4, v5);
    __m128 v8 = _mm_sub_ps(v5, v6);
    __m128 v9 = _mm_add_ps(v6, v7);
    
    /* Call that clobbers vector registers */
    asm volatile("" : : : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                         "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                         "xmm12", "xmm13", "xmm14", "xmm15");
    
    foo();
    
    /* Use all vectors after call */
    __m128 result = _mm_add_ps(v0, v1);
    result = _mm_add_ps(result, v2);
    result = _mm_add_ps(result, v3);
    result = _mm_add_ps(result, v4);
    result = _mm_add_ps(result, v5);
    result = _mm_add_ps(result, v6);
    result = _mm_add_ps(result, v7);
    result = _mm_add_ps(result, v8);
    result = _mm_add_ps(result, v9);
    
    return result;
}
#endif

/* Main function that exercises all tests */
int main(void) {
    int total = 0;
    double fp_total = 0.0;
    
    /* Exercise integer pressure test */
    for (int i = 0; i < 10; i++) {
        total += test_integer_pressure(i, i+1, i+2, i+3);
    }
    
    /* Exercise FP pressure test */
    for (int i = 0; i < 5; i++) {
        fp_total += test_fp_pressure(i * 0.5, i * 0.7, i * 0.9);
    }
    
    /* Exercise mixed pressure test */
    total += test_mixed_pressure(20);
    
    #ifdef __SSE2__
    /* Exercise vector test if available */
    __m128 vec_a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 vec_b = _mm_set_ps(5.0f, 6.0f, 7.0f, 8.0f);
    __m128 vec_c = _mm_set_ps(9.0f, 10.0f, 11.0f, 12.0f);
    __m128 vec_result = test_vector_pressure(vec_a, vec_b, vec_c);
    
    /* Extract result to prevent optimization */
    float vec_floats[4];
    _mm_store_ps(vec_floats, vec_result);
    total += (int)(vec_floats[0] + vec_floats[1] + vec_floats[2] + vec_floats[3]);
    #endif
    
    printf("Result: %d (fp: %f)\n", total, fp_total);
    return total > 0 ? 0 : 1;
}
