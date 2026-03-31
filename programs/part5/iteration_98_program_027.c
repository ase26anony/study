/* test_caller_save.c - Forces GCC to insert save/restore at block ends */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline))

/* External function that clobbers registers */
NOINLINE void foo(void);
NOINLINE void bar(int, double);
NOINLINE void baz(__m128);

/* Helper functions defined in separate compilation unit */
void helper_foo(void) { /* Empty implementation */ }
void helper_bar(int a, double b) { (void)a; (void)b; }
void helper_baz(__m128 v) { (void)v; }

/* Global volatile to prevent optimization */
volatile int global_seed = 42;

/* Test 1: Integer register pressure with call at block end */
NOINLINE int test_integer_pressure(int condition) {
    /* Create many integer live variables across a call */
    volatile int v0 = global_seed;
    register int r0 = v0 + 1;
    register int r1 = r0 * 2;
    register int r2 = r1 + v0;
    register int r3 = r2 - r0;
    register int r4 = r3 * r1;
    register int r5 = r4 / (r0 + 1);
    register int r6 = r5 ^ r2;
    register int r7 = r6 | r3;
    register int r8 = r7 & r4;
    register int r9 = r8 << 2;
    register int r10 = r9 >> 1;
    register int r11 = r10 + r5;
    register int r12 = r11 * r6;
    register int r13 = r12 - r7;
    register int r14 = r13 | r8;
    register int r15 = r14 ^ r9;
    register int r16 = r15 & r10;
    register int r17 = r16 + r11;
    register int r18 = r17 * r12;
    register int r19 = r18 - r13;
    register int r20 = r19 | r14;
    
    /* Use control flow to create basic block ending with call */
    int result;
    if (condition) {
        /* This basic block ends with the call to foo() */
        
        /* Additional pressure before call */
        register int t0 = r0 + r20;
        register int t1 = r1 + r19;
        register int t2 = r2 + r18;
        
        /* Inline asm to clobber caller-saved registers */
        asm volatile("" : : : 
            "rax", "rcx", "rdx", "rsi", "rdi", 
            "r8", "r9", "r10", "r11", "r12",
            "r13", "r14", "r15", "xmm0", "xmm1",
            "xmm2", "xmm3", "xmm4", "xmm5", "xmm6",
            "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
            "xmm12", "xmm13", "xmm14", "xmm15");
        
        /* Call at potential block end */
        foo();
        
        /* Use all variables after call - must be preserved */
        result = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 +
                 r10 + r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18 + r19 + r20 +
                 t0 + t1 + t2;
    } else {
        /* Different path to create CFG complexity */
        result = r0 - r20;
    }
    
    return result;
}

/* Test 2: Floating-point pressure with switch statement */
NOINLINE double test_fp_pressure(int mode) {
    volatile double base = (double)global_seed;
    double d0 = sin(base);
    double d1 = cos(d0);
    double d2 = d0 * d1;
    double d3 = d2 + base;
    double d4 = sin(d3);
    double d5 = cos(d4);
    double d6 = d5 * d4;
    double d7 = d6 + d3;
    double d8 = sin(d7);
    double d9 = cos(d8);
    double d10 = d9 * d8;
    double d11 = d10 + d7;
    double d12 = sin(d11);
    double d13 = cos(d12);
    double d14 = d13 * d12;
    double d15 = d14 + d11;
    double d16 = sin(d15);
    double d17 = cos(d16);
    double d18 = d17 * d16;
    double d19 = d18 + d15;
    
    /* Switch creates multiple basic blocks */
    double result;
    switch (mode % 4) {
        case 0: {
            /* Block ending with call */
            double t0 = d0 + d19;
            double t1 = d1 + d18;
            
            asm volatile("" : : : 
                "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                "xmm12", "xmm13", "xmm14", "xmm15");
            
            bar((int)t0, t1);
            
            result = d0 + d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 +
                    d10 + d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19;
            break;
        }
        case 1:
            result = d0 - d19;
            break;
        case 2:
            result = d1 * d18;
            break;
        default:
            result = d2 / d17;
            break;
    }
    
    return result;
}

/* Test 3: Vector/SIMD pressure with loop unrolling */
NOINLINE __m128 test_vector_pressure(int iterations) {
    __m128 v0 = _mm_set1_ps(1.0f);
    __m128 v1 = _mm_set1_ps(2.0f);
    __m128 v2 = _mm_set1_ps(3.0f);
    __m128 v3 = _mm_set1_ps(4.0f);
    __m128 v4 = _mm_set1_ps(5.0f);
    __m128 v5 = _mm_set1_ps(6.0f);
    __m128 v6 = _mm_set1_ps(7.0f);
    __m128 v7 = _mm_set1_ps(8.0f);
    __m128 v8 = _mm_set1_ps(9.0f);
    __m128 v9 = _mm_set1_ps(10.0f);
    
    /* Partially unrolled loop with call at end of block */
    __m128 accum = _mm_setzero_ps();
    for (int i = 0; i < iterations; i++) {
        /* Multiple vector operations creating pressure */
        __m128 t0 = _mm_add_ps(v0, v1);
        __m128 t1 = _mm_mul_ps(v2, v3);
        __m128 t2 = _mm_sub_ps(v4, v5);
        __m128 t3 = _mm_add_ps(v6, v7);
        __m128 t4 = _mm_mul_ps(v8, v9);
        
        if (i & 1) {
            /* This block ends with call */
            asm volatile("" : : : 
                "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5",
                "ymm6", "ymm7", "ymm8", "ymm9", "ymm10", "ymm11",
                "ymm12", "ymm13", "ymm14", "ymm15");
            
            baz(t0);
            
            accum = _mm_add_ps(accum, t0);
            v0 = _mm_add_ps(v0, t1);
            v1 = _mm_add_ps(v1, t2);
        } else {
            accum = _mm_add_ps(accum, t4);
            v2 = _mm_add_ps(v2, t3);
        }
        
        /* Rotate vectors to keep them all live */
        __m128 tmp = v0;
        v0 = v1; v1 = v2; v2 = v3; v3 = v4; v4 = v5;
        v5 = v6; v6 = v7; v7 = v8; v8 = v9; v9 = tmp;
    }
    
    return accum;
}

/* Test 4: Mixed pressure in nested control flow */
NOINLINE double test_mixed_pressure(int x, int y) {
    volatile int vi = global_seed;
    volatile double vd = (double)global_seed;
    
    /* Integer pressure */
    int i0 = vi + x;
    int i1 = i0 * y;
    int i2 = i1 + vi;
    int i3 = i2 - i0;
    int i4 = i3 * i1;
    int i5 = i4 / (i0 + 1);
    
    /* Floating pressure */
    double d0 = sin(vd);
    double d1 = cos(d0);
    double d2 = d0 * d1;
    double d3 = d2 + vd;
    
    /* Complex control flow with call at merge point */
    double result;
    if (x > 0) {
        if (y > 0) {
            /* Path with register pressure ending in call */
            int t0 = i0 + i5;
            double t1 = d0 + d3;
            
            asm volatile("" : : : 
                "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10",
                "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7");
            
            bar(t0, t1);
            
            result = (double)i0 + d0;
        } else {
            result = (double)i1 + d1;
        }
    } else {
        if (y > 0) {
            result = (double)i2 + d2;
        } else {
            /* Another path ending with call */
            asm volatile("" : : : 
                "r11", "r12", "r13", "r14", "r15",
                "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13");
            
            foo();
            
            result = (double)i3 + d3;
        }
    }
    
    /* Use all variables to keep them live */
    return result + i4 + i5;
}

int main(void) {
    int total = 0;
    double fp_total = 0.0;
    
    /* Call all test functions with different parameters */
    total += test_integer_pressure(1);
    total += test_integer_pressure(0);
    
    fp_total += test_fp_pressure(0);
    fp_total += test_fp_pressure(1);
    fp_total += test_fp_pressure(2);
    fp_total += test_fp_pressure(3);
    
    __m128 vec_result = test_vector_pressure(10);
    float vec_floats[4];
    _mm_storeu_ps(vec_floats, vec_result);
    fp_total += vec_floats[0] + vec_floats[1] + vec_floats[2] + vec_floats[3];
    
    fp_total += test_mixed_pressure(1, 1);
    fp_total += test_mixed_pressure(1, -1);
    fp_total += test_mixed_pressure(-1, 1);
    fp_total += test_mixed_pressure(-1, -1);
    
    /* Use results to prevent dead code elimination */
    printf("Integer total: %d\n", total);
    printf("FP total: %f\n", fp_total);
    
    return (total > 0 && fp_total != 0.0) ? 0 : 1;
}
