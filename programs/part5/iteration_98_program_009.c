/* test_caller_save.c - Forces GCC to insert save/restore at block ends */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline, noipa))

/* External function that clobbers registers */
NOINLINE void foo(void);
NOINLINE void bar(int, double);
NOINLINE void baz(__m128, __m128);

/* Helper to create register pressure */
#define DECL_INT_VARS(n) \
    volatile int int##n##_0, int##n##_1, int##n##_2, int##n##_3, int##n##_4

/* Test 1: Integer register pressure with call at block end */
NOINLINE int test_integer_pressure(int seed) {
    /* Create many integer live values across a call */
    register int r0 = seed + 1;
    register int r1 = r0 * 2;
    register int r2 = r1 + seed;
    register int r3 = r2 - r0;
    register int r4 = r3 * r1;
    register int r5 = r4 / (seed | 1);
    register int r6 = r5 ^ r2;
    register int r7 = r6 << 3;
    register int r8 = r7 >> 1;
    register int r9 = r8 | r3;
    register int r10 = r9 & r4;
    register int r11 = r10 + r5;
    register int r12 = r11 * r6;
    register int r13 = r12 - r7;
    register int r14 = r13 ^ r8;
    register int r15 = r14 | r9;
    register int r16 = r15 & r10;
    register int r17 = r16 + r11;
    register int r18 = r17 * r12;
    register int r19 = r18 - r13;
    register int r20 = r19 ^ r14;
    
    /* Volatile variables that must survive across call */
    volatile int v0 = r0, v1 = r1, v2 = r2, v3 = r3, v4 = r4;
    volatile int v5 = r5, v6 = r6, v7 = r7, v8 = r8, v9 = r9;
    volatile int v10 = r10, v11 = r11, v12 = r12, v13 = r13, v14 = r14;
    
    /* Complex control flow to create basic block ending with call */
    if (seed & 1) {
        /* This basic block ends with the call to foo() */
        int temp = v0 + v1 + v2 + v3 + v4;
        
        /* Inline asm to clobber caller-saved registers */
        asm volatile("" : : : 
            "rax", "rcx", "rdx", "rsi", "rdi", 
            "r8", "r9", "r10", "r11", "r12",
            "r13", "r14", "r15", "xmm0", "xmm1",
            "xmm2", "xmm3", "xmm4", "xmm5", "xmm6",
            "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
            "xmm12", "xmm13", "xmm14", "xmm15");
        
        /* Call at end of basic block */
        foo();
        
        /* Use all variables after call - they must be preserved */
        return v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
               v10 + v11 + v12 + v13 + v14 + r15 + r16 + r17 + r18 + r19 + r20 + temp;
    } else {
        /* Alternative path to create CFG */
        return seed * 2;
    }
}

/* Test 2: Floating-point pressure */
NOINLINE double test_fp_pressure(double seed) {
    /* Many FP calculations */
    double d0 = sin(seed);
    double d1 = cos(d0);
    double d2 = tan(d1);
    double d3 = exp(d2);
    double d4 = log(fabs(d3) + 1.0);
    double d5 = d0 * d1;
    double d6 = d2 / d3;
    double d7 = d4 + d5;
    double d8 = d6 - d7;
    double d9 = sin(d8);
    double d10 = cos(d9);
    double d11 = tan(d10);
    double d12 = exp(d11);
    double d13 = log(fabs(d12) + 1.0);
    double d14 = d9 * d10;
    double d15 = d11 / d12;
    double d16 = d13 + d14;
    double d17 = d15 - d16;
    
    volatile double vd0 = d0, vd1 = d1, vd2 = d2, vd3 = d3, vd4 = d4;
    volatile double vd5 = d5, vd6 = d6, vd7 = d7, vd8 = d8, vd9 = d9;
    
    /* Switch to create multiple basic blocks */
    switch ((int)seed % 4) {
        case 0: {
            /* Block ending with call */
            double sum = vd0 + vd1 + vd2;
            
            /* Clobber FP registers */
            asm volatile("" : : : 
                "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                "xmm12", "xmm13", "xmm14", "xmm15");
            
            bar((int)seed, sum);
            
            return vd0 + vd1 + vd2 + vd3 + vd4 + vd5 + vd6 + vd7 + vd8 + vd9 +
                   d10 + d11 + d12 + d13 + d14 + d15 + d16 + d17 + sum;
        }
        case 1:
            return d0 * 2.0;
        case 2:
            return d1 * 3.0;
        default:
            return d2 * 4.0;
    }
}

/* Test 3: Vector/SIMD pressure */
NOINLINE __m128 test_vector_pressure(float seed) {
    /* Create many vector variables */
    __m128 v0 = _mm_set1_ps(seed);
    __m128 v1 = _mm_add_ps(v0, _mm_set1_ps(1.0f));
    __m128 v2 = _mm_mul_ps(v1, _mm_set1_ps(2.0f));
    __m128 v3 = _mm_sub_ps(v2, v0);
    __m128 v4 = _mm_div_ps(v3, _mm_set1_ps(3.0f));
    __m128 v5 = _mm_add_ps(v4, v1);
    __m128 v6 = _mm_mul_ps(v5, v2);
    __m128 v7 = _mm_sub_ps(v6, v3);
    __m128 v8 = _mm_div_ps(v7, v4);
    __m128 v9 = _mm_add_ps(v8, v5);
    __m128 v10 = _mm_mul_ps(v9, v6);
    __m128 v11 = _mm_sub_ps(v10, v7);
    __m128 v12 = _mm_div_ps(v11, v8);
    
    /* Volatile vectors (GCC may spill these) */
    volatile __m128 vv0 = v0, vv1 = v1, vv2 = v2, vv3 = v3;
    
    /* Loop with call at end of unrolled iteration */
    float result = 0.0f;
    for (int i = 0; i < 3; i++) {
        if (i == 1) {
            /* This block ends with call */
            __m128 temp = _mm_add_ps(vv0, vv1);
            
            /* Clobber vector registers */
            asm volatile("" : : : 
                "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5",
                "ymm6", "ymm7", "ymm8", "ymm9", "ymm10", "ymm11",
                "ymm12", "ymm13", "ymm14", "ymm15");
            
            baz(temp, vv2);
            
            /* Use vectors after call */
            v0 = _mm_add_ps(vv0, vv3);
            result += ((float*)&v0)[0];
        }
        result += ((float*)&vv0)[i % 4];
    }
    
    return _mm_set1_ps(result);
}

/* Test 4: Mixed pressure in nested loops */
NOINLINE int test_mixed_pressure(int iterations) {
    int sum = 0;
    
    /* Unrolled loop to create multiple basic blocks */
    for (int i = 0; i < iterations; i++) {
        if (i & 1) {
            /* Integer pressure */
            int a0 = i * 1, a1 = i * 2, a2 = i * 3, a3 = i * 4, a4 = i * 5;
            int a5 = i * 6, a6 = i * 7, a7 = i * 8, a8 = i * 9, a9 = i * 10;
            volatile int va0 = a0, va1 = a1, va2 = a2;
            
            /* FP pressure */
            double d0 = sin(i), d1 = cos(i), d2 = tan(i);
            volatile double vd0 = d0, vd1 = d1;
            
            /* This block ends with call when i == iterations-1 */
            if (i == iterations - 1) {
                int temp = va0 + va1 + va2;
                
                /* Massive clobber list */
                asm volatile("" : : : 
                    "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                    "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                    "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
                    "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15");
                
                bar(temp, vd0 + vd1);
                
                sum += va0 + va1 + va2 + (int)vd0 + (int)vd1;
            } else {
                sum += a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9;
            }
        }
    }
    
    return sum;
}

int main(void) {
    int total = 0;
    
    /* Run all tests to exercise different register pressure scenarios */
    total += test_integer_pressure(42);
    
    double fp_result = test_fp_pressure(3.14159);
    total += (int)fp_result;
    
    __m128 vec_result = test_vector_pressure(2.5f);
    total += ((int*)&vec_result)[0];
    
    total += test_mixed_pressure(10);
    
    printf("Result: %d\n", total);
    return 0;
}
