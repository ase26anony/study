/* test_caller_save.c */
#include <stdio.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline))

/* External function that clobbers registers */
NOINLINE void foo(void);
NOINLINE void bar(int, double, __m128);
NOINLINE void baz(void);

/* Helper functions defined in separate compilation unit */
void foo(void) { /* Empty but non-inlinable */ }
void bar(int x, double y, __m128 z) { (void)x; (void)y; (void)z; }
void baz(void) { }

/* Test function 1: Integer register pressure with call at block end */
NOINLINE int test_integer_pressure(int seed) {
    volatile int a = seed;
    int result = 0;
    
    /* Create many integer live variables across a call */
    if (a > 0) {
        /* This basic block will end with the call to foo() */
        register int r0 = a + 1;
        register int r1 = r0 * 2;
        register int r2 = r1 + a;
        register int r3 = r2 - r0;
        register int r4 = r3 * r1;
        register int r5 = r4 / (a + 2);
        register int r6 = r5 ^ r2;
        register int r7 = r6 | r3;
        register int r8 = r7 & r4;
        register int r9 = r8 << 2;
        register int r10 = r9 >> 1;
        register int r11 = r10 + r5;
        register int r12 = r11 - r6;
        register int r13 = r12 * r7;
        register int r14 = r13 / (r8 + 1);
        register int r15 = r14 ^ r9;
        register int r16 = r15 | r10;
        register int r17 = r16 & r11;
        register int r18 = r17 << 3;
        register int r19 = r18 >> 2;
        register int r20 = r19 + r12;
        
        /* Clobber many caller-saved registers with inline asm */
        asm volatile("" : : : 
            "rax", "rcx", "rdx", "rsi", "rdi", 
            "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
            "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
            "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
            "xmm12", "xmm13", "xmm14", "xmm15");
        
        /* Call at the end of this basic block */
        foo();
        
        /* Use all variables after call to keep them live */
        result = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 +
                 r10 + r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18 + r19 + r20;
    } else {
        /* Alternative path to create CFG */
        result = -seed;
    }
    
    return result;
}

/* Test function 2: Floating-point pressure with complex control flow */
NOINLINE double test_fp_pressure(double seed) {
    volatile double d = seed;
    double result = 0.0;
    
    /* Switch creates multiple basic blocks */
    switch ((int)d % 4) {
        case 0: {
            /* This case block ends with a call */
            double d0 = sin(d);
            double d1 = cos(d0);
            double d2 = d0 * d1;
            double d3 = d2 + d;
            double d4 = sin(d3);
            double d5 = cos(d4);
            double d6 = d5 * d4;
            double d7 = d6 - d3;
            double d8 = sin(d7);
            double d9 = cos(d8);
            double d10 = d9 + d8;
            double d11 = d10 * d7;
            double d12 = sin(d11);
            double d13 = cos(d12);
            double d14 = d13 - d12;
            double d15 = d14 * d11;
            double d16 = sin(d15);
            double d17 = cos(d16);
            double d18 = d17 + d16;
            double d19 = d18 * d15;
            double d20 = sin(d19);
            
            /* Force use of FP registers */
            asm volatile("" : : : 
                "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                "xmm12", "xmm13", "xmm14", "xmm15");
            
            /* Call at block end */
            bar((int)d, d0, _mm_setzero_ps());
            
            result = d0 + d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 +
                     d10 + d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19 + d20;
            break;
        }
        case 1:
            result = d * 2.0;
            break;
        case 2:
            result = d / 3.0;
            break;
        default:
            result = -d;
            break;
    }
    
    return result;
}

/* Test function 3: Vector/SIMD pressure with loop unrolling */
NOINLINE __m128 test_vector_pressure(float seed) {
    __m128 v0 = _mm_set1_ps(seed);
    __m128 v1 = _mm_add_ps(v0, _mm_set1_ps(1.0f));
    __m128 v2 = _mm_mul_ps(v1, _mm_set1_ps(2.0f));
    __m128 v3 = _mm_sub_ps(v2, v0);
    __m128 v4 = _mm_mul_ps(v3, v1);
    __m128 v5 = _mm_add_ps(v4, v2);
    __m128 v6 = _mm_sub_ps(v5, v3);
    __m128 v7 = _mm_mul_ps(v6, v4);
    __m128 v8 = _mm_add_ps(v7, v5);
    __m128 v9 = _mm_sub_ps(v8, v6);
    __m128 v10 = _mm_mul_ps(v9, v7);
    __m128 v11 = _mm_add_ps(v10, v8);
    __m128 v12 = _mm_sub_ps(v11, v9);
    __m128 v13 = _mm_mul_ps(v12, v10);
    __m128 v14 = _mm_add_ps(v13, v11);
    __m128 v15 = _mm_sub_ps(v14, v12);
    
    /* Partially unrolled loop with call at end of iteration */
    for (int i = 0; i < 2; i++) {
        if (i == 0) {
            /* This block ends with a call */
            __m128 temp = _mm_add_ps(v0, v1);
            
            /* Clobber vector registers */
            asm volatile("" : : : 
                "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5",
                "ymm6", "ymm7", "ymm8", "ymm9", "ymm10", "ymm11",
                "ymm12", "ymm13", "ymm14", "ymm15");
            
            baz();
            
            v0 = _mm_add_ps(v0, temp);
        } else {
            v1 = _mm_add_ps(v1, v0);
        }
    }
    
    return _mm_add_ps(_mm_add_ps(v0, v1), 
                      _mm_add_ps(_mm_add_ps(v2, v3), 
                                 _mm_add_ps(_mm_add_ps(v4, v5), 
                                            _mm_add_ps(_mm_add_ps(v6, v7), 
                                                       _mm_add_ps(_mm_add_ps(v8, v9), 
                                                                  _mm_add_ps(_mm_add_ps(v10, v11), 
                                                                             _mm_add_ps(_mm_add_ps(v12, v13), 
                                                                                        _mm_add_ps(v14, v15))))))));
}

/* Test function 4: Mixed pressure in nested control flow */
NOINLINE double test_mixed_pressure(int i_seed, double d_seed) {
    double result = d_seed;
    
    /* Complex nested if-else structure */
    if (i_seed > 100) {
        if (d_seed < 0.5) {
            /* This innermost block ends with a call */
            int i0 = i_seed + 1;
            int i1 = i0 * 2;
            int i2 = i1 - i_seed;
            double d0 = sin(d_seed);
            double d1 = cos(d_seed);
            __m128 v0 = _mm_set_ps((float)d0, (float)d1, (float)d_seed, 1.0f);
            
            /* Massive clobber list */
            asm volatile("" : : : 
                "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
                "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
                "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7");
            
            foo();
            
            result = i0 + i1 + i2 + d0 + d1 + ((float*)&v0)[0];
        } else {
            result = d_seed * 2.0;
        }
    } else if (i_seed > 50) {
        result = d_seed / 2.0;
    } else {
        result = -d_seed;
    }
    
    return result;
}

int main(void) {
    int int_result = 0;
    double fp_result = 0.0;
    __m128 vec_result;
    float vec_sum[4] = {0};
    
    /* Call all test functions to trigger different pressure scenarios */
    for (int i = 0; i < 10; i++) {
        int_result += test_integer_pressure(i);
        fp_result += test_fp_pressure((double)i);
        vec_result = test_vector_pressure((float)i);
        _mm_storeu_ps(vec_sum, _mm_add_ps(_mm_loadu_ps(vec_sum), vec_result));
        fp_result += test_mixed_pressure(i, (double)i * 0.1);
    }
    
    /* Use results to prevent dead code elimination */
    printf("Results: int=%d, fp=%f, vec=[%f,%f,%f,%f]\n",
           int_result, fp_result,
           vec_sum[0], vec_sum[1], vec_sum[2], vec_sum[3]);
    
    return 0;
}
