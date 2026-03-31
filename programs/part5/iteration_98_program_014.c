/* test_caller_save.c - Program to trigger caller-save insertion at block ends */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline, noipa))

/* External function that clobbers registers */
NOINLINE void foo(void);
NOINLINE void bar(int, double);
NOINLINE void baz(__m128);

/* Helper function in separate compilation unit */
extern void external_func(int, double, __m128);

/* Global volatile to prevent optimization */
volatile int global_counter = 0;

/* ========== Test Case 1: Integer pressure at block end ========== */
NOINLINE int test_integer_pressure(int seed) {
    /* Create massive integer register pressure */
    register int r0  = seed + 1;
    register int r1  = r0 * 2;
    register int r2  = r1 + seed;
    register int r3  = r2 ^ r1;
    register int r4  = r3 * 3;
    register int r5  = r4 - r2;
    register int r6  = r5 & r3;
    register int r7  = r6 | r4;
    register int r8  = r7 << 2;
    register int r9  = r8 >> 1;
    register int r10 = r9 + r0;
    register int r11 = r10 * r1;
    register int r12 = r11 - r2;
    register int r13 = r12 ^ r3;
    register int r14 = r13 | r4;
    register int r15 = r14 & r5;
    register int r16 = r15 << 3;
    register int r17 = r16 >> 2;
    register int r18 = r17 + r6;
    register int r19 = r18 * r7;
    register int r20 = r19 - r8;
    
    /* Volatile variables that must survive across call */
    volatile int v0 = r0;
    volatile int v1 = r1;
    volatile int v2 = r2;
    volatile int v3 = r3;
    volatile int v4 = r4;
    
    /* Complex control flow to create basic block ending with call */
    int result;
    if (seed % 3 == 0) {
        /* This basic block ends with foo() call */
        asm volatile("" : : : 
            "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
            "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
        
        foo();  /* Call at block end - may trigger BB_END update */
        
        result = v0 + v1 + v2 + v3 + v4;
    } else if (seed % 3 == 1) {
        /* Different path */
        result = r20;
    } else {
        /* Another path */
        result = seed * 2;
    }
    
    /* Use all variables after call to keep them live */
    return result + r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10
           + r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18 + r19 + r20;
}

/* ========== Test Case 2: FP pressure with switch statement ========== */
NOINLINE double test_fp_pressure(double seed) {
    /* Create FP register pressure */
    double d0 = sin(seed);
    double d1 = cos(seed);
    double d2 = d0 * d1;
    double d3 = d2 + seed;
    double d4 = exp(d3);
    double d5 = log(fabs(d4) + 1.0);
    double d6 = d5 * d0;
    double d7 = d6 / d1;
    double d8 = d7 - d2;
    double d9 = d8 + d3;
    double d10 = d9 * d4;
    double d11 = d10 / d5;
    double d12 = d11 + d6;
    double d13 = d12 * d7;
    double d14 = d13 / d8;
    double d15 = d14 + d9;
    double d16 = d15 * d10;
    double d17 = d16 / d11;
    double d18 = d17 + d12;
    double d19 = d18 * d13;
    double d20 = d19 / d14;
    
    volatile double vd0 = d0;
    volatile double vd1 = d1;
    volatile double vd2 = d2;
    
    /* Switch creates multiple basic blocks */
    int choice = (int)seed % 5;
    double result;
    
    switch (choice) {
        case 0:
            /* This block ends with bar() call */
            asm volatile("" : : : 
                "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                "xmm12", "xmm13", "xmm14", "xmm15");
            
            bar((int)seed, d0);  /* Call at block end */
            result = vd0 + vd1 + vd2;
            break;
            
        case 1:
            result = d20;
            break;
            
        case 2:
            result = d15;
            break;
            
        case 3:
            result = d10;
            break;
            
        default:
            result = d5;
            break;
    }
    
    return result + d0 + d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10
           + d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19 + d20;
}

/* ========== Test Case 3: Vector pressure in loop ========== */
NOINLINE __m128 test_vector_pressure(float seed) {
    /* Create vector register pressure */
    __m128 v0 = _mm_set1_ps(seed);
    __m128 v1 = _mm_set1_ps(seed + 1.0f);
    __m128 v2 = _mm_add_ps(v0, v1);
    __m128 v3 = _mm_mul_ps(v2, v0);
    __m128 v4 = _mm_sub_ps(v3, v1);
    __m128 v5 = _mm_div_ps(v4, v2);
    __m128 v6 = _mm_set1_ps(seed * 2.0f);
    __m128 v7 = _mm_add_ps(v5, v6);
    __m128 v8 = _mm_mul_ps(v7, v3);
    __m128 v9 = _mm_sub_ps(v8, v4);
    __m128 v10 = _mm_div_ps(v9, v5);
    __m128 v11 = _mm_set1_ps(seed * 3.0f);
    __m128 v12 = _mm_add_ps(v10, v11);
    __m128 v13 = _mm_mul_ps(v12, v7);
    __m128 v14 = _mm_sub_ps(v13, v8);
    __m128 v15 = _mm_div_ps(v14, v9);
    
    volatile __m128 vv0 = v0;
    volatile __m128 vv1 = v1;
    
    /* Partially unrolled loop with call at end of iteration */
    __m128 result = v0;
    for (int i = 0; i < 3; i++) {
        if (i == 1) {
            /* This block ends with baz() call */
            asm volatile("" : : : 
                "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5",
                "ymm6", "ymm7", "ymm8", "ymm9", "ymm10", "ymm11",
                "ymm12", "ymm13", "ymm14", "ymm15");
            
            baz(v0);  /* Call at block end inside loop */
            
            result = _mm_add_ps(result, vv0);
        } else {
            result = _mm_add_ps(result, vv1);
        }
        
        /* More computations to increase pressure */
        v0 = _mm_add_ps(v0, v1);
        v1 = _mm_add_ps(v1, v2);
    }
    
    return _mm_add_ps(result, _mm_add_ps(v2, _mm_add_ps(v3, 
           _mm_add_ps(v4, _mm_add_ps(v5, _mm_add_ps(v6,
           _mm_add_ps(v7, _mm_add_ps(v8, _mm_add_ps(v9,
           _mm_add_ps(v10, _mm_add_ps(v11, _mm_add_ps(v12,
           _mm_add_ps(v13, _mm_add_ps(v14, v15))))))))))))));
}

/* ========== Test Case 4: Mixed pressure with nested calls ========== */
NOINLINE double test_mixed_pressure(int i_seed, double d_seed) {
    /* Integer pressure */
    int i0 = i_seed;
    int i1 = i0 * 2;
    int i2 = i1 + 1;
    int i3 = i2 ^ i0;
    int i4 = i3 * 3;
    int i5 = i4 - i1;
    
    /* FP pressure */
    double d0 = d_seed;
    double d1 = d0 * 2.0;
    double d2 = d1 + 1.0;
    double d3 = sin(d2);
    double d4 = cos(d3);
    
    /* Vector pressure */
    __m128 v0 = _mm_set1_ps((float)d0);
    __m128 v1 = _mm_set1_ps((float)d1);
    
    volatile int vi0 = i0;
    volatile double vd0 = d0;
    volatile __m128 vv0 = v0;
    
    /* Complex control flow with call at end of two different blocks */
    double result;
    
    if (i_seed > 0) {
        if (d_seed > 0.5) {
            /* This block ends with external_func call */
            asm volatile("" : : : 
                "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                "r8", "r9", "r10", "r11", "r12",
                "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                "xmm6", "xmm7", "xmm8", "xmm9");
            
            external_func(i0, d0, v0);  /* Call at block end */
            
            result = d0 + vi0 + ((float*)&vv0)[0];
        } else {
            result = d1 + i1;
        }
    } else {
        if (d_seed < -0.5) {
            result = d2 + i2;
        } else {
            /* Another block ending with call */
            asm volatile("" : : : 
                "rax", "rcx", "rdx", "rsi", "rdi",
                "xmm0", "xmm1", "xmm2", "xmm3");
            
            bar(i3, d3);  /* Call at block end */
            
            result = d3 + i3;
        }
    }
    
    /* Use all variables */
    return result + i0 + i1 + i2 + i3 + i4 + i5
           + d0 + d1 + d2 + d3 + d4
           + ((float*)&v0)[0] + ((float*)&v1)[0]
           + ((float*)&vv0)[0];
}

/* ========== Main driver ========== */
int main(void) {
    int total = 0;
    float fsum = 0.0f;
    
    /* Run all test cases multiple times with different seeds */
    for (int i = 0; i < 100; i++) {
        total += test_integer_pressure(i);
        
        double d_result = test_fp_pressure((double)i * 0.1);
        total += (int)d_result;
        
        __m128 v_result = test_vector_pressure((float)i * 0.1f);
        float v_elems[4];
        _mm_store_ps(v_elems, v_result);
        fsum += v_elems[0] + v_elems[1] + v_elems[2] + v_elems[3];
        
        double m_result = test_mixed_pressure(i, (double)i * 0.01);
        total += (int)m_result;
        
        global_counter++;  /* Prevent optimization of loop */
    }
    
    printf("Total: %d, Float sum: %f\n", total, fsum);
    return total > 0 ? 0 : 1;
}
