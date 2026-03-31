/* test_caller_save.c - Forces GCC to insert save/restore instructions at call sites */
#include <stdio.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline, noipa))

/* External function that clobbers registers */
NOINLINE void foo(void);
NOINLINE void bar(int, double);
NOINLINE void baz(__m128);

/* Helper functions in separate compilation unit */
extern void external_func1(void);
extern void external_func2(int);
extern double external_func3(double);

/* Global volatile to prevent optimization */
volatile int global_seed = 42;

/* Test 1: Integer register pressure with call at block end */
NOINLINE int test_integer_pressure(int cond) {
    /* Create many integer live variables across a call */
    register int r0  = global_seed + 1;
    register int r1  = r0 * 2;
    register int r2  = r1 + global_seed;
    register int r3  = r2 ^ r1;
    register int r4  = r3 * 3;
    register int r5  = r4 - r2;
    register int r6  = r5 >> 1;
    register int r7  = r6 | r4;
    register int r8  = r7 & 0xFFFF;
    register int r9  = r8 + r3;
    register int r10 = r9 * 7;
    register int r11 = r10 - r5;
    register int r12 = r11 ^ r8;
    register int r13 = r12 + 12345;
    register int r14 = r13 * 2;
    register int r15 = r14 - 6789;
    int r16 = r15 + r0;
    int r17 = r16 * r1;
    int r18 = r17 ^ r2;
    int r19 = r18 | r3;
    int r20 = r19 & r4;
    int r21 = r20 + r5;
    int r22 = r21 * r6;
    int r23 = r22 - r7;
    int r24 = r23 ^ r8;
    int r25 = r24 | r9;
    int r26 = r25 + r10;
    int r27 = r26 * r11;
    int r28 = r27 - r12;
    int r29 = r28 ^ r13;
    int r30 = r29 | r14;
    
    /* Use control flow to create basic block ending with call */
    if (cond > 0) {
        /* Inline asm to clobber caller-saved integer registers */
        asm volatile("" : : : 
            "rax", "rcx", "rdx", "rsi", "rdi", 
            "r8", "r9", "r10", "r11", "r12",
            "r13", "r14", "r15", "xmm0", "xmm1",
            "xmm2", "xmm3", "xmm4", "xmm5", "xmm6",
            "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
            "xmm12", "xmm13", "xmm14", "xmm15");
        
        /* Call at potential block end */
        foo();
        
        /* Use all variables after call to keep them live */
        return r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 +
               r10 + r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18 + r19 +
               r20 + r21 + r22 + r23 + r24 + r25 + r26 + r27 + r28 + r29 + r30;
    } else {
        /* Different path to create CFG complexity */
        return cond;
    }
}

/* Test 2: Floating-point pressure with switch statement */
NOINLINE double test_float_pressure(int mode) {
    volatile double v0 = sin(global_seed * 0.1);
    volatile double v1 = cos(v0 * 2.0);
    volatile double v2 = v0 + v1;
    volatile double v3 = v1 * v2;
    volatile double v4 = sin(v3);
    volatile double v5 = cos(v4);
    volatile double v6 = v4 + v5;
    volatile double v7 = v5 * v6;
    volatile double v8 = sin(v7);
    volatile double v9 = cos(v8);
    volatile double v10 = v8 + v9;
    volatile double v11 = v9 * v10;
    volatile double v12 = sin(v11);
    volatile double v13 = cos(v12);
    volatile double v14 = v12 + v13;
    volatile double v15 = v13 * v14;
    double v16 = v14 + v15;
    double v17 = v15 * v16;
    double v18 = sin(v17);
    double v19 = cos(v18);
    double v20 = v18 + v19;
    
    /* Switch creates multiple basic blocks */
    double result = 0.0;
    switch (mode % 4) {
        case 0:
            /* Call at end of this case's block */
            bar(global_seed, v0);
            result = v0 + v2 + v4 + v6 + v8 + v10 + v12 + v14 + v16 + v18 + v20;
            break;
        case 1:
            result = v1 + v3 + v5 + v7 + v9 + v11 + v13 + v15 + v17 + v19;
            break;
        case 2:
            /* Another call site */
            external_func3(v2);
            result = v0 * v1 * v2 * v3 * v4;
            break;
        default:
            result = v20;
            break;
    }
    
    /* Use variables after switch */
    return result + v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
           v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
}

/* Test 3: Vector register pressure with loop unrolling */
NOINLINE __m128 test_vector_pressure(int iterations) {
    __m128 vec0 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 vec1 = _mm_set_ps(5.0f, 6.0f, 7.0f, 8.0f);
    __m128 vec2 = _mm_add_ps(vec0, vec1);
    __m128 vec3 = _mm_mul_ps(vec0, vec1);
    __m128 vec4 = _mm_sub_ps(vec2, vec3);
    __m128 vec5 = _mm_set_ps(9.0f, 10.0f, 11.0f, 12.0f);
    __m128 vec6 = _mm_add_ps(vec4, vec5);
    __m128 vec7 = _mm_mul_ps(vec4, vec5);
    __m128 vec8 = _mm_set_ps(13.0f, 14.0f, 15.0f, 16.0f);
    __m128 vec9 = _mm_add_ps(vec6, vec8);
    __m128 vec10 = _mm_mul_ps(vec7, vec8);
    __m128 vec11 = _mm_set_ps(17.0f, 18.0f, 19.0f, 20.0f);
    __m128 vec12 = _mm_add_ps(vec9, vec11);
    __m128 vec13 = _mm_mul_ps(vec10, vec11);
    __m128 vec14 = _mm_set_ps(21.0f, 22.0f, 23.0f, 24.0f);
    __m128 vec15 = _mm_add_ps(vec12, vec14);
    
    /* Partially unrolled loop with call at end of iteration */
    __m128 accum = _mm_setzero_ps();
    for (int i = 0; i < iterations; i++) {
        /* Manual unrolling */
        accum = _mm_add_ps(accum, vec0);
        accum = _mm_add_ps(accum, vec1);
        accum = _mm_add_ps(accum, vec2);
        
        if (i % 3 == 0) {
            /* Call inside loop, potentially at block end */
            baz(accum);
            
            accum = _mm_add_ps(accum, vec3);
            accum = _mm_add_ps(accum, vec4);
        } else {
            accum = _mm_add_ps(accum, vec5);
            accum = _mm_add_ps(accum, vec6);
        }
        
        accum = _mm_add_ps(accum, vec7);
        accum = _mm_add_ps(accum, vec8);
        
        /* Another potential block-ending call */
        if (i == iterations - 1) {
            external_func1();
        }
    }
    
    /* Use all vectors after loop */
    __m128 sum = _mm_add_ps(vec0, vec1);
    sum = _mm_add_ps(sum, vec2);
    sum = _mm_add_ps(sum, vec3);
    sum = _mm_add_ps(sum, vec4);
    sum = _mm_add_ps(sum, vec5);
    sum = _mm_add_ps(sum, vec6);
    sum = _mm_add_ps(sum, vec7);
    sum = _mm_add_ps(sum, vec8);
    sum = _mm_add_ps(sum, vec9);
    sum = _mm_add_ps(sum, vec10);
    sum = _mm_add_ps(sum, vec11);
    sum = _mm_add_ps(sum, vec12);
    sum = _mm_add_ps(sum, vec13);
    sum = _mm_add_ps(sum, vec14);
    sum = _mm_add_ps(sum, vec15);
    
    return _mm_add_ps(accum, sum);
}

/* Test 4: Mixed pressure in nested control flow */
NOINLINE double test_mixed_pressure(int x, int y) {
    /* Integer pressure */
    int i0 = x + 1, i1 = i0 * 2, i2 = i1 + y, i3 = i2 ^ i1;
    int i4 = i3 * 3, i5 = i4 - i2, i6 = i5 >> 1, i7 = i6 | i4;
    int i8 = i7 & 0xFF, i9 = i8 + i3, i10 = i9 * 7;
    
    /* Floating pressure */
    double f0 = sin(x * 0.01), f1 = cos(f0), f2 = f0 + f1;
    double f3 = f1 * f2, f4 = sin(f3), f5 = cos(f4);
    double f6 = f4 + f5, f7 = f5 * f6, f8 = sin(f7);
    
    /* Complex nested if-else creating multiple block ends */
    double result = 0.0;
    if (x > 0) {
        if (y > 0) {
            /* Call at end of this block */
            bar(i0, f0);
            result = f0 + f2 + f4 + f6 + f8;
        } else {
            external_func2(i1);
            result = f1 + f3 + f5 + f7;
        }
        
        /* Use integers after call */
        result += i0 + i2 + i4 + i6 + i8 + i10;
    } else {
        if (y < 0) {
            foo();
            result = f0 * f2 * f4;
        } else {
            result = f1 * f3 * f5;
        }
        
        result += i1 + i3 + i5 + i7 + i9;
    }
    
    /* Force all variables to be used */
    volatile int check = i0 + i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10;
    (void)check;
    
    return result + f0 + f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8;
}

/* Main driver that calls all tests */
int main(void) {
    int total = 0;
    double sum = 0.0;
    
    /* Call each test multiple times with different parameters */
    for (int i = 0; i < 10; i++) {
        total += test_integer_pressure(i);
        sum += test_float_pressure(i);
        
        __m128 vec_result = test_vector_pressure(5);
        float vec_sum[4];
        _mm_store_ps(vec_sum, vec_result);
        sum += vec_sum[0] + vec_sum[1] + vec_sum[2] + vec_sum[3];
        
        sum += test_mixed_pressure(i, i * 2);
    }
    
    /* Prevent dead code elimination */
    printf("Result: %d, %f\n", total, sum);
    return (total > 0 && sum != 0.0) ? 0 : 1;
}

/* Dummy implementations of called functions */
NOINLINE void foo(void) {
    /* Empty but non-inlinable */
    asm volatile("" : : : "memory");
}

NOINLINE void bar(int a, double b) {
    /* Use arguments to prevent optimization */
    volatile int x = a;
    volatile double y = b;
    (void)x;
    (void)y;
}

NOINLINE void baz(__m128 v) {
    /* Use vector argument */
    volatile __m128 temp = v;
    (void)temp;
}
