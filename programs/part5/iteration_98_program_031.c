/* test_caller_save.c - Forces GCC to insert save/restore instructions at call sites */
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
NOINLINE void baz(__m128, __m128);

/* Helper to use variables after calls */
volatile int sink_int;
volatile double sink_double;
volatile __m128 sink_vec;

/* Test 1: Integer register pressure with call at block end */
NOINLINE int test_integer_pressure(int a, int b, int c) {
    /* Create many integer live values across a call */
    volatile int v0 = a;
    register int r0 = v0 + 1;
    register int r1 = r0 * b;
    register int r2 = r1 - c;
    register int r3 = r2 ^ a;
    register int r4 = r3 | b;
    register int r5 = r4 & c;
    register int r6 = r5 << 2;
    register int r7 = r6 >> 1;
    register int r8 = r7 + r0;
    register int r9 = r8 * r1;
    register int r10 = r9 - r2;
    register int r11 = r10 ^ r3;
    register int r12 = r11 | r4;
    register int r13 = r12 & r5;
    register int r14 = r13 << 3;
    register int r15 = r14 >> 2;
    register int r16 = r15 + r6;
    register int r17 = r16 * r7;
    register int r18 = r17 - r8;
    register int r19 = r18 ^ r9;
    register int r20 = r19 | r10;
    
    /* Complex control flow to create basic blocks */
    if (a > b) {
        /* This call is at the end of a basic block */
        foo();  /* Non-inline call */
        
        /* Use all variables after call */
        sink_int = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 +
                  r10 + r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18 + r19 + r20;
        return sink_int;
    } else {
        /* Different path to create CFG complexity */
        register int t0 = r0 * 2;
        register int t1 = t0 + r1;
        register int t2 = t1 * r2;
        return t2;
    }
}

/* Test 2: Floating-point pressure with switch statement */
NOINLINE double test_fp_pressure(double x, double y) {
    double result = 0.0;
    
    /* Many FP live values */
    volatile double v0 = x;
    double d0 = sin(v0);
    double d1 = cos(d0);
    double d2 = d0 * d1 + y;
    double d3 = d2 * d2 - d0;
    double d4 = d3 / (d1 + 1.0);
    double d5 = d4 * d3;
    double d6 = d5 + d2;
    double d7 = d6 - d1;
    double d8 = d7 * d0;
    double d9 = d8 / d3;
    double d10 = d9 + d4;
    double d11 = d10 * d5;
    double d12 = d11 - d6;
    double d13 = d12 / d7;
    double d14 = d13 * d8;
    double d15 = d14 + d9;
    double d16 = d15 - d10;
    double d17 = d16 * d11;
    double d18 = d17 / d12;
    double d19 = d18 + d13;
    double d20 = d19 * d14;
    
    /* Switch creates multiple basic blocks */
    switch ((int)x % 4) {
        case 0:
            /* Call at end of this case's block */
            bar((int)d0, d1);
            result = d0 + d1 + d2 + d3 + d4 + d5;
            break;
        case 1:
            result = d6 + d7 + d8 + d9 + d10;
            bar((int)d6, d7);
            break;
        case 2:
            /* More register pressure before call */
            asm volatile("" : : : 
                "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                "xmm12", "xmm13", "xmm14", "xmm15");
            bar((int)d11, d12);
            result = d11 + d12 + d13 + d14 + d15;
            break;
        default:
            result = d16 + d17 + d18 + d19 + d20;
            break;
    }
    
    sink_double = result;
    return sink_double;
}

/* Test 3: Vector register pressure with loop unrolling */
NOINLINE __m128 test_vector_pressure(float *data) {
    __m128 vec0 = _mm_load_ps(data);
    __m128 vec1 = _mm_load_ps(data + 4);
    __m128 vec2 = _mm_add_ps(vec0, vec1);
    __m128 vec3 = _mm_mul_ps(vec0, vec1);
    __m128 vec4 = _mm_sub_ps(vec2, vec3);
    __m128 vec5 = _mm_add_ps(vec4, vec0);
    __m128 vec6 = _mm_mul_ps(vec5, vec1);
    __m128 vec7 = _mm_sub_ps(vec6, vec2);
    __m128 vec8 = _mm_add_ps(vec7, vec3);
    __m128 vec9 = _mm_mul_ps(vec8, vec4);
    __m128 vec10 = _mm_sub_ps(vec9, vec5);
    __m128 vec11 = _mm_add_ps(vec10, vec6);
    __m128 vec12 = _mm_mul_ps(vec11, vec7);
    __m128 vec13 = _mm_sub_ps(vec12, vec8);
    __m128 vec14 = _mm_add_ps(vec13, vec9);
    __m128 vec15 = _mm_mul_ps(vec14, vec10);
    
    /* Partially unrolled loop with call at end */
    for (int i = 0; i < 2; i++) {
        /* Manual unrolling to create block ending with call */
        if (i == 0) {
            /* Clobber vector registers */
            asm volatile("" : : : 
                "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5",
                "ymm6", "ymm7", "ymm8", "ymm9", "ymm10", "ymm11",
                "ymm12", "ymm13", "ymm14", "ymm15");
            
            /* Call at block end */
            baz(vec0, vec1);
            
            __m128 t0 = _mm_add_ps(vec2, vec3);
            vec0 = _mm_add_ps(vec0, t0);
        } else {
            __m128 t1 = _mm_add_ps(vec4, vec5);
            vec1 = _mm_add_ps(vec1, t1);
        }
    }
    
    sink_vec = _mm_add_ps(_mm_add_ps(vec0, vec1), 
                         _mm_add_ps(vec2, vec3));
    return sink_vec;
}

/* Test 4: Mixed pressure with nested control flow */
NOINLINE double test_mixed_pressure(int n, double x) {
    double total = 0.0;
    
    for (int i = 0; i < n; i++) {
        /* Integer pressure */
        register int i0 = i * 2;
        register int i1 = i0 + 1;
        register int i2 = i1 * i;
        register int i3 = i2 ^ i0;
        register int i4 = i3 | i1;
        register int i5 = i4 & i2;
        
        /* FP pressure */
        double d0 = sin(x + i);
        double d1 = cos(d0);
        double d2 = d0 * d1;
        double d3 = d2 + x;
        double d4 = d3 * d0;
        
        /* Complex if-else chain */
        if (i % 3 == 0) {
            /* Call at end of this block */
            bar(i0, d0);
            total += d0 + i0;
        } else if (i % 3 == 1) {
            total += d1 + i1;
            /* Another call at block end */
            bar(i1, d1);
        } else {
            /* Even more register pressure */
            register int i6 = i5 << 1;
            register int i7 = i6 + i4;
            double d5 = d4 * d2;
            double d6 = d5 + d3;
            
            /* Clobber both integer and FP registers */
            asm volatile("" : : : 
                "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                "xmm12", "xmm13", "xmm14", "xmm15");
            
            bar(i7, d6);
            total += d6 + i7;
        }
    }
    
    return total;
}

/* Main driver that calls all tests */
int main(void) {
    float vec_data[16];
    for (int i = 0; i < 16; i++) {
        vec_data[i] = (float)i;
    }
    
    /* Call all test functions to trigger different save/restore patterns */
    int r1 = test_integer_pressure(100, 200, 300);
    double r2 = test_fp_pressure(3.14159, 2.71828);
    __m128 r3 = test_vector_pressure(vec_data);
    double r4 = test_mixed_pressure(10, 1.234);
    
    /* Use results to prevent optimization */
    printf("Results: %d, %f, %f\n", r1, r2, r4);
    
    float r3_vals[4];
    _mm_store_ps(r3_vals, r3);
    printf("Vector: %f %f %f %f\n", r3_vals[0], r3_vals[1], r3_vals[2], r3_vals[3]);
    
    return 0;
}
