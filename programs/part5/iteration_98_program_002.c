/* test_caller_save.c - Program to trigger caller-save insertion at block ends */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline, noipa))

/* External function that clobbers registers - defined in separate file */
NOINLINE void foo(void);
NOINLINE void bar(int, double);
NOINLINE void baz(__m128, __m256);

/* Helper to use variables after calls */
volatile int use_result;

/* Test 1: Integer register pressure with call at end of basic block */
NOINLINE int test_integer_pressure(int a, int b, int c) {
    /* Create many integer live values that must survive across call */
    volatile int v0 = a + 1;
    register int r1 = v0 * 2 + b;
    volatile int v2 = r1 - c;
    register int r3 = v2 * 3;
    volatile int v4 = r3 / 2;
    register int r5 = v4 + a;
    volatile int v6 = r5 - b;
    register int r7 = v6 * c;
    volatile int v8 = r7 + 1;
    register int r9 = v8 / 2;
    volatile int v10 = r9 * 3;
    register int r11 = v10 - a;
    volatile int v12 = r11 + b;
    register int r13 = v12 * c;
    volatile int v14 = r13 / 4;
    register int r15 = v14 + 5;
    volatile int v16 = r15 - 6;
    register int r17 = v16 * 7;
    volatile int v18 = r17 + 8;
    register int r19 = v18 / 9;
    volatile int v20 = r19 * 10;
    register int r21 = v20 - 11;
    volatile int v22 = r21 + 12;
    register int r23 = v22 * 13;
    volatile int v24 = r23 / 14;
    register int r25 = v24 + 15;
    volatile int v26 = r25 - 16;
    register int r27 = v26 * 17;
    volatile int v28 = r27 + 18;
    register int r29 = v28 / 19;
    
    /* Complex control flow to create basic block ending with call */
    if (a > b) {
        /* This block ends with the call to foo() */
        int result = v0 + r1 + v2 + r3 + v4 + r5 + v6 + r7 + v8 + r9 +
                    v10 + r11 + v12 + r13 + v14 + r15 + v16 + r17 + v18 + r19 +
                    v20 + r21 + v22 + r23 + v24 + r25 + v26 + r27 + v28 + r29;
        
        /* Inline assembly to clobber caller-saved integer registers */
        asm volatile("" : : : 
            "rax", "rcx", "rdx", "rsi", "rdi", 
            "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
            "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
            "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15");
        
        /* Call at end of basic block - forces save/restore insertion */
        foo();
        
        /* Use all variables after call to keep them live */
        use_result = result + v0 + r29;
        return result;
    } else {
        /* Different path to create CFG complexity */
        return a + b + c;
    }
}

/* Test 2: Floating-point pressure with switch statement */
NOINLINE double test_fp_pressure(double x, double y, int mode) {
    /* Many FP variables that must survive across call */
    volatile double d0 = sin(x);
    double d1 = cos(y);
    volatile double d2 = d0 * d1;
    double d3 = d2 + x;
    volatile double d4 = d3 - y;
    double d5 = d4 * 2.0;
    volatile double d6 = sin(d5);
    double d7 = cos(d6);
    volatile double d8 = d7 * 3.14159;
    double d9 = d8 / 2.71828;
    volatile double d10 = exp(d9);
    double d11 = log(fabs(d10) + 1.0);
    volatile double d12 = d11 * y;
    double d13 = d12 + x;
    volatile double d14 = pow(d13, 2.0);
    double d15 = sqrt(d14);
    volatile double d16 = d15 * 0.5;
    double d17 = d16 + 1.0;
    volatile double d18 = sin(d17);
    double d19 = cos(d18);
    
    /* Switch creates multiple basic blocks */
    double result = 0.0;
    switch (mode % 4) {
        case 0:
            /* Call at end of this case block */
            result = d0 + d1 + d2 + d3 + d4;
            bar(1, result);
            /* Use variables after call */
            use_result = (int)(d19 * 1000);
            break;
        case 1:
            result = d5 + d6 + d7 + d8 + d9;
            bar(2, result);
            use_result = (int)(d18 * 1000);
            break;
        case 2:
            /* This case has the call at block end */
            result = d10 + d11 + d12 + d13 + d14;
            /* Clobber FP registers */
            asm volatile("" : : : 
                "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", 
                "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                "xmm12", "xmm13", "xmm14", "xmm15");
            bar(3, result);
            use_result = (int)(d17 * 1000);
            break;
        case 3:
            result = d15 + d16 + d17 + d18 + d19;
            bar(4, result);
            use_result = (int)(d16 * 1000);
            break;
    }
    
    return result;
}

/* Test 3: Vector register pressure with loop unrolling */
NOINLINE __m128 test_vector_pressure(float *data, int n) {
    /* Many vector variables */
    __m128 v0 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 v1 = _mm_set_ps(5.0f, 6.0f, 7.0f, 8.0f);
    __m128 v2 = _mm_add_ps(v0, v1);
    __m128 v3 = _mm_mul_ps(v0, v1);
    __m128 v4 = _mm_sub_ps(v2, v3);
    __m128 v5 = _mm_set_ps(9.0f, 10.0f, 11.0f, 12.0f);
    __m128 v6 = _mm_add_ps(v4, v5);
    __m128 v7 = _mm_mul_ps(v4, v5);
    __m128 v8 = _mm_sub_ps(v6, v7);
    __m128 v9 = _mm_set_ps(13.0f, 14.0f, 15.0f, 16.0f);
    __m128 v10 = _mm_add_ps(v8, v9);
    __m128 v11 = _mm_mul_ps(v8, v9);
    __m128 v12 = _mm_sub_ps(v10, v11);
    
    /* Partially unrolled loop with call at end of iteration */
    __m128 sum = _mm_setzero_ps();
    for (int i = 0; i < n; i += 4) {
        if (i + 4 <= n) {
            __m128 data_vec = _mm_loadu_ps(&data[i]);
            __m128 temp = _mm_add_ps(data_vec, v0);
            temp = _mm_add_ps(temp, v1);
            temp = _mm_add_ps(temp, v2);
            temp = _mm_add_ps(temp, v3);
            temp = _mm_add_ps(temp, v4);
            temp = _mm_add_ps(temp, v5);
            
            /* Call inside loop - creates block ending with call */
            if (i % 8 == 0) {
                /* Clobber vector registers */
                asm volatile("" : : : 
                    "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5",
                    "ymm6", "ymm7", "ymm8", "ymm9", "ymm10", "ymm11",
                    "ymm12", "ymm13", "ymm14", "ymm15");
                baz(v12, _mm256_setzero_ps());
            }
            
            sum = _mm_add_ps(sum, temp);
            v0 = _mm_add_ps(v0, _mm_set1_ps(0.1f));
        }
    }
    
    return _mm_add_ps(sum, v12);
}

/* Test 4: Mixed pressure in nested control flow */
NOINLINE int test_mixed_pressure(int a, double b, float c) {
    volatile int iv0 = a * 2;
    double dv0 = b * 3.14;
    __m128 vv0 = _mm_set1_ps(c);
    
    int result = 0;
    
    /* Complex nested if-else to create interesting CFG */
    if (a > 0) {
        if (b > 0.0) {
            volatile int iv1 = iv0 + 10;
            double dv1 = dv0 * 2.0;
            __m128 vv1 = _mm_add_ps(vv0, _mm_set1_ps(1.0f));
            
            for (int i = 0; i < 3; i++) {
                /* Call at end of loop body block */
                if (i == 1) {
                    /* Maximum pressure here */
                    volatile int iv2 = iv1 * i;
                    double dv2 = dv1 + (double)i;
                    __m128 vv2 = _mm_mul_ps(vv1, _mm_set1_ps((float)i));
                    
                    /* Clobber everything */
                    asm volatile("" : : : 
                        "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
                        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
                        "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15");
                    
                    foo();
                    
                    result += iv2 + (int)dv2;
                    float tmp[4];
                    _mm_storeu_ps(tmp, vv2);
                    result += (int)tmp[0];
                }
            }
        } else {
            bar(a, b);
        }
    } else {
        result = -1;
    }
    
    return result;
}

/* Main function that exercises all tests */
int main(void) {
    int total = 0;
    double dtotal = 0.0;
    float data[64];
    
    /* Initialize data */
    for (int i = 0; i < 64; i++) {
        data[i] = (float)i * 0.1f;
    }
    
    /* Run all tests to create various pressure scenarios */
    total += test_integer_pressure(10, 20, 30);
    total += test_integer_pressure(5, 15, 25);
    
    dtotal += test_fp_pressure(1.0, 2.0, 0);
    dtotal += test_fp_pressure(3.0, 4.0, 2);
    
    __m128 vec_result = test_vector_pressure(data, 64);
    float vec_sum[4];
    _mm_storeu_ps(vec_sum, vec_result);
    total += (int)(vec_sum[0] + vec_sum[1] + vec_sum[2] + vec_sum[3]);
    
    total += test_mixed_pressure(100, 3.14159, 2.71828f);
    
    printf("Result: %d (%.2f)\n", total, dtotal);
    return total > 0 ? 0 : 1;
}
