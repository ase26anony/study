/* test_caller_save.c - Forces GCC to insert save/restore instructions at call sites */

#include <math.h>
#include <stdint.h>
#include <stdio.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline))

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
NOINLINE int test_int_pressure(int seed) {
    volatile int a = seed;
    volatile int b = seed * 2;
    volatile int c = seed + 1;
    
    /* Create many integer live variables */
    register int r0 = a + 1;
    register int r1 = r0 * 2 + b;
    register int r2 = r1 - c;
    register int r3 = r2 ^ a;
    register int r4 = r3 | b;
    register int r5 = r4 & c;
    register int r6 = r5 << 2;
    register int r7 = r6 >> 1;
    register int r8 = r7 + r0;
    register int r9 = r8 - r1;
    register int r10 = r9 * r2;
    register int r11 = r10 / (r3 + 1);
    register int r12 = r11 % (r4 + 1);
    register int r13 = r12 ^ r5;
    register int r14 = r13 | r6;
    register int r15 = r14 & r7;
    register int r16 = r15 << 3;
    register int r17 = r16 >> 2;
    register int r18 = r17 + r8;
    register int r19 = r18 - r9;
    register int r20 = r19 * r10;
    
    /* Complex control flow to create basic blocks */
    if (a > 0) {
        /* This creates a basic block ending with the call */
        CLOBBER_REGS();  /* Force compiler to assume registers clobbered */
        foo();  /* Non-inline call at potential block end */
        
        /* Use all variables after call to keep them live */
        r0 += r20;
        r1 += r19;
        r2 += r18;
        r3 += r17;
        r4 += r16;
        r5 += r15;
        r6 += r14;
        r7 += r13;
        r8 += r12;
        r9 += r11;
        r10 += r20;
    } else {
        /* Alternative path to create CFG complexity */
        r0 = r1 = r2 = 0;
    }
    
    /* Force use of all variables */
    return r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 +
           r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18 + r19 + r20;
}

/* Test 2: Floating-point register pressure */
NOINLINE double test_fp_pressure(double seed) {
    volatile double v = seed;
    
    /* Many FP variables */
    double f0 = sin(v);
    double f1 = cos(v);
    double f2 = f0 * f1;
    double f3 = f2 + v;
    double f4 = f3 * f0;
    double f5 = f4 / f1;
    double f6 = f5 - f2;
    double f7 = f6 + f3;
    double f8 = f7 * f4;
    double f9 = f8 / f5;
    double f10 = f9 - f6;
    double f11 = f10 + f7;
    double f12 = f11 * f8;
    double f13 = f12 / f9;
    double f14 = f13 - f10;
    double f15 = f14 + f11;
    double f16 = f15 * f12;
    double f17 = f16 / f13;
    double f18 = f17 - f14;
    double f19 = f18 + f15;
    double f20 = f19 * f16;
    
    /* Switch statement to create multiple basic blocks */
    int choice = (int)v % 3;
    double result = 0;
    
    switch (choice) {
        case 0:
            /* Call at end of this case's block */
            CLOBBER_REGS();
            result = baz(f0, f1);  /* Non-inline FP call */
            f2 += result;
            f3 += f2;
            break;
        case 1:
            result = f4 + f5;
            break;
        case 2:
            result = f6 * f7;
            break;
    }
    
    /* Use all FP variables */
    return f0 + f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 + f10 +
           f11 + f12 + f13 + f14 + f15 + f16 + f17 + f18 + f19 + f20 + result;
}

/* Test 3: Mixed integer/FP with loop unrolling */
NOINLINE double test_mixed_pressure(int iter) {
    double acc = 0.0;
    volatile int vi = iter;
    
    /* Partially unrolled loop */
    for (int i = 0; i < iter; i++) {
        /* Many live variables inside loop */
        int i0 = vi + i;
        int i1 = i0 * 2;
        int i2 = i1 - i;
        int i3 = i2 ^ i0;
        int i4 = i3 | i1;
        
        double d0 = sin(acc);
        double d1 = cos(acc);
        double d2 = d0 * d1;
        double d3 = d2 + acc;
        double d4 = d3 * d0;
        
        /* Call inside loop - may be at block end after unrolling */
        if (i % 4 == 0) {
            CLOBBER_REGS();
            bar(i0, d0);  /* Non-inline call */
            
            /* Keep variables live */
            i1 += i2;
            d1 += d2;
        }
        
        acc += i0 + i1 + i2 + i3 + i4 + d0 + d1 + d2 + d3 + d4;
        
        /* More variables to increase pressure */
        int i5 = i4 << 1;
        int i6 = i5 >> 1;
        double d5 = d4 / 2.0;
        double d6 = d5 * 3.0;
        
        acc += i5 + i6 + d5 + d6;
    }
    
    return acc;
}

/* Test 4: Vector pressure (if supported) */
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
    
    /* Complex if-else to create block structure */
    if (a > b) {
        /* Call at potential block end */
        CLOBBER_REGS();
        foo();
        
        v1 = _mm_add_pd(v1, v10);
        v2 = _mm_add_pd(v2, v9);
    } else {
        v1 = _mm_sub_pd(v1, v10);
        v2 = _mm_sub_pd(v2, v9);
    }
    
    /* Use all vectors */
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

/* Main driver that calls all tests */
int main(void) {
    int int_result;
    double fp_result, mixed_result;
    
    /* Call integer pressure test */
    int_result = test_int_pressure(42);
    printf("Integer test result: %d\n", int_result);
    
    /* Call FP pressure test */
    fp_result = test_fp_pressure(3.14159);
    printf("FP test result: %f\n", fp_result);
    
    /* Call mixed pressure test */
    mixed_result = test_mixed_pressure(10);
    printf("Mixed test result: %f\n", mixed_result);
    
#ifdef __SSE2__
    /* Call vector test if supported */
    __m128d vec_result = test_vector_pressure(1.0, 2.0);
    double vec_arr[2];
    _mm_store_pd(vec_arr, vec_result);
    printf("Vector test result: %f, %f\n", vec_arr[0], vec_arr[1]);
#endif
    
    /* Ensure results are used */
    return (int)(int_result + fp_result + mixed_result) % 256;
}
