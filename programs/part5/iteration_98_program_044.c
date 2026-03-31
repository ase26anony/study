/* test_caller_save.c - Forces caller-save register spills at block ends */

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline, noipa))

/* External function declarations */
NOINLINE void foo(void);
NOINLINE void bar(int, double);
NOINLINE double baz(double, double);

/* Helper to clobber many registers */
#define CLOBBER_REGS() \
    asm volatile("" : : : \
        "rax", "rbx", "rcx", "rdx", "rsi", "rdi", \
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", \
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", \
        "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", \
        "xmm12", "xmm13", "xmm14", "xmm15")

/* Test 1: Integer register pressure with call at block end */
NOINLINE int test_integer_pressure(int seed) {
    volatile int v = seed; /* Prevent optimization */
    
    /* Create many integer live values across a call */
    register int r0  = v + 1;
    register int r1  = r0 * 2;
    register int r2  = r1 + v;
    register int r3  = r2 - r0;
    register int r4  = r3 * r1;
    register int r5  = r4 ^ r2;
    register int r6  = r5 | r3;
    register int r7  = r6 & r4;
    register int r8  = r7 << 2;
    register int r9  = r8 >> 1;
    register int r10 = r9 + r5;
    register int r11 = r10 * 3;
    register int r12 = r11 - r6;
    register int r13 = r12 ^ r7;
    register int r14 = r13 | r8;
    register int r15 = r14 & r9;
    register int r16 = r15 << 3;
    register int r17 = r16 >> 2;
    register int r18 = r17 + r10;
    register int r19 = r18 * 5;
    register int r20 = r19 - r11;
    register int r21 = r20 ^ r12;
    register int r22 = r21 | r13;
    register int r23 = r22 & r14;
    register int r24 = r23 << 1;
    register int r25 = r24 >> 1;
    
    /* Complex control flow to create basic block ending with call */
    if (v > 0) {
        /* This basic block ends with the call to foo() */
        CLOBBER_REGS(); /* Force many registers to be considered live */
        foo(); /* Non-inlineable call at block end */
        
        /* Use all variables after call - must be preserved */
        return r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 +
               r10 + r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18 + r19 +
               r20 + r21 + r22 + r23 + r24 + r25;
    } else {
        /* Alternative path - creates CFG merge point */
        return v * 2;
    }
}

/* Test 2: Floating-point pressure with switch statement */
NOINLINE double test_fp_pressure(double seed) {
    volatile double v = seed;
    
    /* Many FP calculations */
    double f0 = sin(v);
    double f1 = cos(v);
    double f2 = f0 * f1;
    double f3 = f2 + v;
    double f4 = f3 * f0;
    double f5 = f4 / f1;
    double f6 = sin(f5);
    double f7 = cos(f6);
    double f8 = f7 * f2;
    double f9 = f8 + f3;
    double f10 = f9 * f4;
    double f11 = f10 / f5;
    double f12 = sin(f11);
    double f13 = cos(f12);
    double f14 = f13 * f8;
    double f15 = f14 + f9;
    double f16 = f15 * f10;
    double f17 = f16 / f11;
    double f18 = sin(f17);
    double f19 = cos(f18);
    double f20 = f19 * f14;
    
    /* Switch creates multiple basic blocks */
    int choice = (int)v % 4;
    double result = 0;
    
    switch (choice) {
        case 0:
            CLOBBER_REGS();
            bar(choice, f0); /* Call at end of case block */
            result = f0 + f2 + f4 + f6 + f8 + f10;
            break;
        case 1:
            result = f1 + f3 + f5 + f7 + f9 + f11;
            CLOBBER_REGS();
            bar(choice, f1);
            break;
        case 2:
            /* This case ends with call at block end */
            CLOBBER_REGS();
            bar(choice, f2);
            result = f12 + f14 + f16 + f18 + f20;
            break;
        default:
            result = f13 + f15 + f17 + f19;
            break;
    }
    
    /* Use all FP values after switch */
    return result + f0 + f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 +
                   f10 + f11 + f12 + f13 + f14 + f15 + f16 + f17 + f18 + f19 + f20;
}

/* Test 3: Vector pressure with loop unrolling */
#ifdef __SSE2__
#include <emmintrin.h>
NOINLINE __m128d test_vector_pressure(double a, double b) {
    /* Many vector variables */
    __m128d v0 = _mm_set_pd(a, b);
    __m128d v1 = _mm_add_pd(v0, v0);
    __m128d v2 = _mm_mul_pd(v1, v0);
    __m128d v3 = _mm_sub_pd(v2, v1);
    __m128d v4 = _mm_add_pd(v3, v2);
    __m128d v5 = _mm_mul_pd(v4, v3);
    __m128d v6 = _mm_sub_pd(v5, v4);
    __m128d v7 = _mm_add_pd(v6, v5);
    __m128d v8 = _mm_mul_pd(v7, v6);
    __m128d v9 = _mm_sub_pd(v8, v7);
    __m128d v10 = _mm_add_pd(v9, v8);
    
    /* Partially unrolled loop with call at end of iteration */
    double sum = 0.0;
    for (int i = 0; i < 4; i++) {
        /* Each iteration creates a basic block */
        if (i & 1) {
            CLOBBER_REGS();
            double r = baz(a, b); /* Call at potential block end */
            __m128d temp = _mm_set1_pd(r);
            v0 = _mm_add_pd(v0, temp);
            sum += r;
        } else {
            v1 = _mm_add_pd(v1, v0);
        }
        
        /* More vector operations */
        v2 = _mm_mul_pd(v2, _mm_set1_pd(i + 1));
        v3 = _mm_add_pd(v3, v2);
    }
    
    /* Combine all vectors */
    __m128d result = _mm_add_pd(v0, v1);
    result = _mm_add_pd(result, v2);
    result = _mm_add_pd(result, v3);
    result = _mm_add_pd(result, v4);
    result = _mm_add_pd(result, v5);
    result = _mm_add_pd(result, v6);
    result = _mm_add_pd(result, v7);
    result = _mm_add_pd(result, v8);
    result = _mm_add_pd(result, v9);
    result = _mm_add_pd(result, v10);
    
    return result;
}
#endif

/* Test 4: Mixed pressure in nested control flow */
NOINLINE double test_mixed_pressure(int mode, double x) {
    volatile int vi = mode;
    volatile double vd = x;
    
    /* Integer variables */
    int i0 = vi + 1, i1 = i0 * 2, i2 = i1 + vi, i3 = i2 - i0;
    int i4 = i3 * i1, i5 = i4 ^ i2, i6 = i5 | i3, i7 = i6 & i4;
    
    /* Floating-point variables */
    double d0 = sin(vd), d1 = cos(vd), d2 = d0 * d1, d3 = d2 + vd;
    double d4 = d3 * d0, d5 = d4 / d1, d6 = sin(d5), d7 = cos(d6);
    
    /* Complex nested if-else creating multiple block ends */
    double result = 0;
    if (vi > 100) {
        if (vd > 0.5) {
            CLOBBER_REGS();
            bar(i0, d0); /* Call at nested block end */
            result = d0 + d2 + d4 + d6;
        } else {
            result = d1 + d3 + d5 + d7;
        }
        result += i0 + i2 + i4 + i6;
    } else if (vi > 50) {
        CLOBBER_REGS();
        foo(); /* Another call at block end */
        result = d0 * 2 + d1 * 3;
        result += i1 + i3 + i5 + i7;
    } else {
        /* Multiple calls in sequence */
        CLOBBER_REGS();
        bar(i0, d0);
        CLOBBER_REGS();
        bar(i1, d1);
        result = (d0 + d1) * (i0 + i1);
    }
    
    /* Use all variables */
    return result + i0 + i1 + i2 + i3 + i4 + i5 + i6 + i7 +
                   d0 + d1 + d2 + d3 + d4 + d5 + d6 + d7;
}

/* Main driver */
int main(void) {
    double total = 0.0;
    
    /* Run all tests with different parameters */
    total += test_integer_pressure(42);
    total += test_fp_pressure(3.14159);
    
    #ifdef __SSE2__
    __m128d vec = test_vector_pressure(1.0, 2.0);
    double vec_sum;
    _mm_store_sd(&vec_sum, vec);
    total += vec_sum;
    #endif
    
    total += test_mixed_pressure(75, 2.71828);
    
    /* Prevent dead code elimination */
    volatile double sink = total;
    printf("Result: %f\n", sink);
    
    return 0;
}
