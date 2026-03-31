/* test_caller_save.c */
#include <stdio.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline, noipa))

/* External function declarations */
NOINLINE void foo(void);
NOINLINE void bar(int, double);
NOINLINE void baz(__m128, __m256);

/* Helper functions to create register pressure */
NOINLINE int helper_int(int a, int b) {
    return a + b;
}

NOINLINE double helper_double(double a, double b) {
    return a * b;
}

/* Function 1: Heavy integer register pressure with call at block end */
NOINLINE int test_integer_pressure(int seed) {
    volatile int v0 = seed;
    register int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9;
    register int r10, r11, r12, r13, r14, r15, r16, r17, r18, r19;
    register int r20, r21, r22, r23, r24, r25, r26, r27, r28, r29;
    
    /* Create complex dependency chain */
    r0 = v0 + 1;
    r1 = r0 * 2;
    r2 = r1 + v0;
    r3 = r2 - r0;
    r4 = r3 * r1;
    r5 = r4 / (r2 + 1);
    r6 = r5 << 2;
    r7 = r6 ^ r3;
    r8 = r7 | r4;
    r9 = r8 & r5;
    r10 = r9 + r6;
    r11 = r10 * r7;
    r12 = r11 - r8;
    r13 = r12 / (r9 + 1);
    r14 = r13 << 1;
    r15 = r14 ^ r10;
    r16 = r15 | r11;
    r17 = r16 & r12;
    r18 = r17 + r13;
    r19 = r18 * r14;
    r20 = r19 - r15;
    r21 = r20 / (r16 + 1);
    r22 = r21 << 3;
    r23 = r22 ^ r17;
    r24 = r23 | r18;
    r25 = r24 & r19;
    r26 = r25 + r20;
    r27 = r26 * r21;
    r28 = r27 - r22;
    r29 = r28 / (r23 + 1);
    
    /* Use inline assembly to clobber caller-saved registers */
    asm volatile("" : : : 
        "rax", "rcx", "rdx", "rsi", "rdi", 
        "r8", "r9", "r10", "r11",
        "xmm0", "xmm1", "xmm2", "xmm3", 
        "xmm4", "xmm5", "xmm6", "xmm7",
        "xmm8", "xmm9", "xmm10", "xmm11",
        "xmm12", "xmm13", "xmm14", "xmm15");
    
    /* Create basic block structure where call is at the end */
    if (seed % 2 == 0) {
        /* This path creates a basic block ending with foo() */
        int temp = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 +
                  r10 + r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18 + r19 +
                  r20 + r21 + r22 + r23 + r24 + r25 + r26 + r27 + r28 + r29;
        
        /* Call at the end of this basic block */
        foo();
        
        /* Use all variables after call to keep them live */
        return temp + v0;
    } else {
        /* Alternative path to create CFG */
        return r29;
    }
}

/* Function 2: Heavy floating-point register pressure */
NOINLINE double test_float_pressure(double seed) {
    volatile double v0 = seed;
    double d0, d1, d2, d3, d4, d5, d6, d7, d8, d9;
    double d10, d11, d12, d13, d14, d15, d16, d17, d18, d19;
    double d20, d21, d22, d23, d24, d25, d26, d27, d28, d29;
    
    /* Create FP dependency chain */
    d0 = sin(v0);
    d1 = cos(d0);
    d2 = sin(d1);
    d3 = cos(d2);
    d4 = sin(d3);
    d5 = cos(d4);
    d6 = sin(d5);
    d7 = cos(d6);
    d8 = sin(d7);
    d9 = cos(d8);
    d10 = sin(d9);
    d11 = cos(d10);
    d12 = sin(d11);
    d13 = cos(d12);
    d14 = sin(d13);
    d15 = cos(d14);
    d16 = sin(d15);
    d17 = cos(d16);
    d18 = sin(d17);
    d19 = cos(d18);
    d20 = sin(d19);
    d21 = cos(d20);
    d22 = sin(d21);
    d23 = cos(d22);
    d24 = sin(d23);
    d25 = cos(d24);
    d26 = sin(d25);
    d27 = cos(d26);
    d28 = sin(d27);
    d29 = cos(d28);
    
    /* Use switch to create complex CFG */
    int choice = (int)seed % 4;
    double result = 0.0;
    
    switch (choice) {
        case 0:
            /* Call at end of this case's basic block */
            bar((int)d0, d1);
            result = d0 + d1 + d2 + d3;
            break;
        case 1:
            result = d4 + d5 + d6 + d7;
            bar((int)d4, d5);
            break;
        case 2:
            result = d8 + d9 + d10 + d11;
            /* Call in the middle, not at block end */
            bar((int)d8, d9);
            result += d12 + d13;
            break;
        default:
            /* Call at end of default block */
            result = d14 + d15 + d16 + d17;
            bar((int)d14, d15);
    }
    
    /* Use all variables to keep them live */
    return result + d18 + d19 + d20 + d21 + d22 + d23 + 
           d24 + d25 + d26 + d27 + d28 + d29 + v0;
}

/* Function 3: Vector register pressure with AVX */
NOINLINE __m256 test_vector_pressure(float seed) {
    __m128 v0 = _mm_set1_ps(seed);
    __m128 v1 = _mm_set1_ps(seed + 1.0f);
    __m128 v2 = _mm_set1_ps(seed + 2.0f);
    __m128 v3 = _mm_set1_ps(seed + 3.0f);
    __m128 v4 = _mm_set1_ps(seed + 4.0f);
    __m128 v5 = _mm_set1_ps(seed + 5.0f);
    __m128 v6 = _mm_set1_ps(seed + 6.0f);
    __m128 v7 = _mm_set1_ps(seed + 7.0f);
    __m128 v8 = _mm_set1_ps(seed + 8.0f);
    __m128 v9 = _mm_set1_ps(seed + 9.0f);
    
    __m256 w0 = _mm256_set1_ps(seed);
    __m256 w1 = _mm256_set1_ps(seed + 10.0f);
    __m256 w2 = _mm256_set1_ps(seed + 20.0f);
    __m256 w3 = _mm256_set1_ps(seed + 30.0f);
    __m256 w4 = _mm256_set1_ps(seed + 40.0f);
    __m256 w5 = _mm256_set1_ps(seed + 50.0f);
    __m256 w6 = _mm256_set1_ps(seed + 60.0f);
    __m256 w7 = _mm256_set1_ps(seed + 70.0f);
    
    /* Create vector computations */
    __m128 sum128 = _mm_add_ps(v0, v1);
    sum128 = _mm_add_ps(sum128, v2);
    sum128 = _mm_add_ps(sum128, v3);
    sum128 = _mm_add_ps(sum128, v4);
    sum128 = _mm_add_ps(sum128, v5);
    sum128 = _mm_add_ps(sum128, v6);
    sum128 = _mm_add_ps(sum128, v7);
    sum128 = _mm_add_ps(sum128, v8);
    sum128 = _mm_add_ps(sum128, v9);
    
    __m256 sum256 = _mm256_add_ps(w0, w1);
    sum256 = _mm256_add_ps(sum256, w2);
    sum256 = _mm256_add_ps(sum256, w3);
    sum256 = _mm256_add_ps(sum256, w4);
    sum256 = _mm256_add_ps(sum256, w5);
    sum256 = _mm256_add_ps(sum256, w6);
    sum256 = _mm256_add_ps(sum256, w7);
    
    /* Use loop with partial unrolling - call at end of unrolled block */
    float result[8] = {0};
    
    for (int i = 0; i < 4; i++) {
        /* Partially unrolled loop body */
        if (i == 2) {
            /* Call at end of this basic block */
            baz(v0, w0);
            /* Continue using vectors after call */
            sum256 = _mm256_add_ps(sum256, w0);
        }
        
        /* More vector operations */
        w0 = _mm256_add_ps(w0, w1);
        w1 = _mm256_add_ps(w1, w2);
    }
    
    /* Mix all vector types */
    __m256 final = _mm256_add_ps(
        sum256,
        _mm256_set_m128(sum128, sum128)
    );
    
    return final;
}

/* Function 4: Mixed register pressure in nested control flow */
NOINLINE double test_mixed_pressure(int i_seed, double d_seed) {
    volatile int vi = i_seed;
    volatile double vd = d_seed;
    
    /* Integer variables */
    int i0 = vi + 1, i1 = i0 * 2, i2 = i1 + 3, i3 = i2 * 4;
    int i4 = i3 - 5, i5 = i4 / 6, i6 = i5 << 2, i7 = i6 ^ 0xFF;
    int i8 = i7 | 0xAA, i9 = i8 & 0x55, i10 = i9 + i0;
    
    /* Floating-point variables */
    double d0 = sin(vd), d1 = cos(d0), d2 = exp(d1), d3 = log(fabs(d2) + 1.0);
    double d4 = d3 * 2.0, d5 = d4 / 3.0, d6 = sqrt(d5), d7 = pow(d6, 2.0);
    double d8 = d7 + d0, d9 = d8 - d1, d10 = d9 * d2;
    
    /* Complex nested if-else structure */
    double result = 0.0;
    
    if (vi > 0) {
        if (vd < 0.5) {
            /* Call at end of this innermost block */
            foo();
            result = d0 + d1;
        } else {
            result = d2 + d3;
            bar(i0, d4);
        }
        
        /* Use integer variables */
        result += i0 + i1 + i2;
    } else {
        if (vd > 0.5) {
            result = d4 + d5;
            /* Another call site */
            bar(i3, d6);
        } else {
            /* Call at end of this block */
            result = d7 + d8;
            foo();
        }
        
        result += i3 + i4 + i5;
    }
    
    /* Use all remaining variables */
    return result + i6 + i7 + i8 + i9 + i10 + d9 + d10;
}

/* Main function that exercises all test cases */
int main(void) {
    int total = 0;
    double sum = 0.0;
    
    /* Test 1: Integer pressure */
    for (int i = 0; i < 10; i++) {
        total += test_integer_pressure(i);
    }
    
    /* Test 2: Floating-point pressure */
    for (int i = 0; i < 10; i++) {
        sum += test_float_pressure(i * 0.1);
    }
    
    /* Test 3: Vector pressure */
    float vector_sum = 0.0f;
    for (int i = 0; i < 5; i++) {
        __m256 vec_result = test_vector_pressure(i * 0.2f);
        float temp[8];
        _mm256_storeu_ps(temp, vec_result);
        for (int j = 0; j < 8; j++) {
            vector_sum += temp[j];
        }
    }
    
    /* Test 4: Mixed pressure */
    for (int i = 0; i < 10; i++) {
        sum += test_mixed_pressure(i, i * 0.1);
    }
    
    /* Ensure results are used */
    printf("Results: total=%d, sum=%f, vector_sum=%f\n", 
           total, sum, vector_sum);
    
    return (total > 0 && sum > 0.0) ? 0 : 1;
}
