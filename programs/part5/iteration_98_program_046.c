/* test_caller_save.c - Forces caller-save register spilling at block ends */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline, noipa))

/* External function declarations */
NOINLINE void foo(void);
NOINLINE void bar(int, double);
NOINLINE int baz(void);
NOINLINE double complex_math(double, double);

/* Helper functions in separate compilation unit */
extern void external_func1(void);
extern void external_func2(int);
extern double external_func3(double, double);

/* Global volatile variables to prevent optimization */
volatile int global_counter = 0;
volatile double global_double = 3.14159;

/* Function 1: Heavy integer register pressure with call at block end */
NOINLINE int test_integer_pressure(int seed) {
    /* Create many integer live variables across a call */
    register int r0 = seed + 1;
    volatile int v1 = r0 * 2;
    register int r2 = v1 + seed;
    volatile int v3 = r2 ^ 0xABCD;
    register int r4 = v3 - seed;
    volatile int v5 = r4 | 0x1234;
    register int r6 = v5 * 3;
    volatile int v7 = r6 / 2;
    register int r8 = v7 + v1;
    volatile int v9 = r8 << 2;
    register int r10 = v9 ^ v3;
    volatile int v11 = r10 + r4;
    register int r12 = v11 * v5;
    volatile int v13 = r12 - r6;
    register int r14 = v13 | v7;
    volatile int v15 = r14 ^ r8;
    register int r16 = v15 + v9;
    volatile int v17 = r16 * r10;
    register int r18 = v17 - v11;
    volatile int v19 = r18 | r12;
    register int r20 = v19 ^ v13;
    volatile int v21 = r20 + r14;
    register int r22 = v21 * v15;
    volatile int v23 = r22 - r16;
    register int r24 = v23 | v17;
    volatile int v25 = r24 ^ v18;
    
    /* Complex control flow to create basic block ending with call */
    int result = 0;
    if (seed % 3 == 0) {
        /* This block ends with the call to foo() */
        asm volatile("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                     "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
        
        /* Call with many live integer variables */
        foo();
        
        /* Use all variables after call to keep them live */
        result = r0 + v1 + r2 + v3 + r4 + v5 + r6 + v7 + r8 + v9 +
                 r10 + v11 + r12 + v13 + r14 + v15 + r16 + v17 + r18 +
                 v19 + r20 + v21 + r22 + v23 + r24 + v25;
    } else if (seed % 3 == 1) {
        result = baz();
    } else {
        result = seed * 2;
    }
    
    return result;
}

/* Function 2: Heavy floating-point register pressure */
NOINLINE double test_float_pressure(double a, double b) {
    /* Many floating-point variables */
    volatile double d0 = sin(a);
    register double d1 = cos(b);
    volatile double d2 = d0 * d1;
    register double d3 = tan(a + b);
    volatile double d4 = d2 + d3;
    register double d5 = exp(d4);
    volatile double d6 = log(fabs(d5));
    register double d7 = d6 * 2.0;
    volatile double d8 = d7 - d0;
    register double d9 = d8 / d1;
    volatile double d10 = pow(d9, 2.0);
    register double d11 = sqrt(d10);
    volatile double d12 = d11 + d2;
    register double d13 = d12 * d3;
    volatile double d14 = d13 - d4;
    register double d15 = d14 / d5;
    volatile double d16 = sin(d15);
    register double d17 = cos(d16);
    volatile double d18 = d17 * d6;
    register double d19 = d18 + d7;
    volatile double d20 = d19 - d8;
    
    /* Switch statement to create complex CFG */
    double result = 0.0;
    switch ((int)a % 4) {
        case 0:
            /* Call at end of this basic block */
            asm volatile("" : : : "xmm0", "xmm1", "xmm2", "xmm3", 
                         "xmm4", "xmm5", "xmm6", "xmm7",
                         "xmm8", "xmm9", "xmm10", "xmm11",
                         "xmm12", "xmm13", "xmm14", "xmm15");
            
            bar((int)a, b);
            
            result = d0 + d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 +
                    d10 + d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 +
                    d19 + d20;
            break;
            
        case 1:
            result = complex_math(a, b);
            break;
            
        case 2:
            result = external_func3(d0, d1);
            break;
            
        default:
            result = a * b;
            break;
    }
    
    return result;
}

/* Function 3: Vector/SIMD register pressure */
NOINLINE __m128 test_vector_pressure(float f1, float f2, float f3, float f4) {
    /* Many vector variables */
    __m128 v0 = _mm_set_ps(f1, f2, f3, f4);
    volatile __m128 v1 = _mm_add_ps(v0, v0);
    __m128 v2 = _mm_mul_ps(v1, _mm_set1_ps(2.0f));
    volatile __m128 v3 = _mm_sub_ps(v2, v0);
    __m128 v4 = _mm_div_ps(v3, _mm_set1_ps(3.0f));
    volatile __m128 v5 = _mm_add_ps(v4, v1);
    __m128 v6 = _mm_mul_ps(v5, v2);
    volatile __m128 v7 = _mm_sub_ps(v6, v3);
    __m128 v8 = _mm_div_ps(v7, v4);
    volatile __m128 v9 = _mm_add_ps(v8, v5);
    __m128 v10 = _mm_mul_ps(v9, v6);
    
    /* Loop with partial unrolling - call at end of unrolled block */
    __m128 accum = _mm_setzero_ps();
    for (int i = 0; i < 8; i++) {
        if (i & 1) {
            /* This block ends with external call */
            asm volatile("" : : : "ymm0", "ymm1", "ymm2", "ymm3",
                         "ymm4", "ymm5", "ymm6", "ymm7",
                         "ymm8", "ymm9", "ymm10", "ymm11",
                         "ymm12", "ymm13", "ymm14", "ymm15");
            
            external_func1();
            
            accum = _mm_add_ps(accum, v0);
            v0 = _mm_add_ps(v0, v1);
        } else {
            accum = _mm_add_ps(accum, v2);
            v2 = _mm_add_ps(v2, v3);
        }
        
        /* Rotate vectors to keep them all live */
        __m128 temp = v0;
        v0 = v1; v1 = v2; v2 = v3; v3 = v4;
        v4 = v5; v5 = v6; v6 = v7; v7 = v8;
        v8 = v9; v9 = v10; v10 = temp;
    }
    
    return accum;
}

/* Function 4: Mixed register pressure in nested control flow */
NOINLINE double test_mixed_pressure(int mode, double x) {
    double result = x;
    
    /* Complex nested if-else structure */
    if (mode > 0) {
        if (mode < 10) {
            /* Integer pressure */
            int i0 = mode * 2, i1 = i0 + 1, i2 = i1 * 3, i3 = i2 - 1;
            int i4 = i3 ^ 0xFF, i5 = i4 << 2, i6 = i5 >> 1, i7 = i6 | 0xAA;
            volatile int vi8 = i7 + i0, vi9 = i1 * i2, vi10 = i3 ^ i4;
            
            /* Float pressure */
            double f0 = sin(x), f1 = cos(x), f2 = tan(x);
            volatile double vf3 = f0 * f1, vf4 = f1 + f2, vf5 = f2 / f0;
            
            /* This block ends with call */
            asm volatile("" : : : 
                "rax", "rbx", "rcx", "rdx",
                "xmm0", "xmm1", "xmm2", "xmm3",
                "xmm4", "xmm5", "xmm6", "xmm7");
            
            external_func2(mode);
            
            result = (i0 + i1 + i2 + i3 + i4 + i5 + i6 + i7 + 
                     vi8 + vi9 + vi10) * (f0 + f1 + f2 + vf3 + vf4 + vf5);
        } else {
            result = complex_math(x, x * 2);
        }
    } else if (mode < 0) {
        result = test_float_pressure(x, x / 2);
    } else {
        result = 1.0;
    }
    
    return result;
}

/* Main function that exercises all test cases */
int main(void) {
    int total = 0;
    double sum = 0.0;
    
    /* Test integer pressure with different seeds */
    for (int i = 0; i < 10; i++) {
        total += test_integer_pressure(i + global_counter);
    }
    
    /* Test floating-point pressure */
    for (int i = 0; i < 8; i++) {
        sum += test_float_pressure(i * 0.5, i * 0.25);
    }
    
    /* Test vector pressure */
    __m128 vec_result = test_vector_pressure(1.0f, 2.0f, 3.0f, 4.0f);
    float vec_floats[4];
    _mm_store_ps(vec_floats, vec_result);
    sum += vec_floats[0] + vec_floats[1] + vec_floats[2] + vec_floats[3];
    
    /* Test mixed pressure */
    for (int i = -5; i < 5; i++) {
        sum += test_mixed_pressure(i, i * 0.1);
    }
    
    /* Use results to prevent dead code elimination */
    printf("Total: %d, Sum: %f\n", total, sum);
    
    return (total > 0 && sum > 0) ? 0 : 1;
}
