/* test_caller_save.c - Forces caller-save register spilling at block ends */

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline))

/* Helper function in separate compilation unit */
NOINLINE void foo(void);
NOINLINE void bar(int, double);
NOINLINE void baz(__m128, __m256);

/* Global volatile to prevent optimization */
volatile int global_seed = 42;

/* ========== Integer Register Pressure ========== */
NOINLINE int test_integer_pressure(int cond) {
    /* Create massive integer register pressure */
    register int r0  = global_seed + 1;
    register int r1  = r0 * 2;
    register int r2  = r1 + global_seed;
    register int r3  = r2 ^ r1;
    register int r4  = r3 << 2;
    register int r5  = r4 - r0;
    register int r6  = r5 | r2;
    register int r7  = r6 & r3;
    register int r8  = r7 >> 1;
    register int r9  = r8 + r4;
    register int r10 = r9 * 3;
    register int r11 = r10 - r5;
    register int r12 = r11 ^ r6;
    register int r13 = r12 << 3;
    register int r14 = r13 | r7;
    register int r15 = r14 & r8;
    register int r16 = r15 + r9;
    register int r17 = r16 * 5;
    register int r18 = r17 - r10;
    register int r19 = r18 ^ r11;
    register int r20 = r19 << 1;
    
    /* Volatile variables that must survive across call */
    volatile int v0 = r0;
    volatile int v1 = r1;
    volatile int v2 = r2;
    volatile int v3 = r3;
    volatile int v4 = r4;
    volatile int v5 = r5;
    
    /* Complex control flow to create basic block ending with call */
    if (cond > 0) {
        /* More computations to increase pressure */
        int t0 = r20 + r12;
        int t1 = t0 * r13;
        int t2 = t1 ^ r14;
        int t3 = t2 | r15;
        int t4 = t3 & r16;
        int t5 = t4 + r17;
        int t6 = t5 * r18;
        int t7 = t6 ^ r19;
        
        /* Inline assembly to clobber caller-saved registers */
        asm volatile("" : : : 
            "rax", "rcx", "rdx", "rsi", "rdi",
            "r8", "r9", "r10", "r11"
        );
        
        /* Call at potential block end */
        foo();  /* This call should trigger save/restore insertion */
        
        /* Use all variables after call */
        return v0 + v1 + v2 + v3 + v4 + v5 + 
               t0 + t1 + t2 + t3 + t4 + t5 + t6 + t7;
    } else {
        /* Alternative path - creates another basic block */
        return r0 + r1 + r2;
    }
}

/* ========== Floating Point Register Pressure ========== */
NOINLINE double test_fp_pressure(int mode) {
    /* Create massive FP register pressure */
    double d0  = sin(global_seed * 0.1);
    double d1  = cos(d0);
    double d2  = d0 + d1;
    double d3  = d1 * d2;
    double d4  = sin(d2);
    double d5  = cos(d3);
    double d6  = d3 + d4;
    double d7  = d4 * d5;
    double d8  = sin(d5);
    double d9  = cos(d6);
    double d10 = d6 + d7;
    double d11 = d7 * d8;
    double d12 = sin(d8);
    double d13 = cos(d9);
    double d14 = d9 + d10;
    double d15 = d10 * d11;
    double d16 = sin(d11);
    double d17 = cos(d12);
    double d18 = d12 + d13;
    double d19 = d13 * d14;
    double d20 = sin(d14);
    
    volatile double vd0 = d0;
    volatile double vd1 = d1;
    volatile double vd2 = d2;
    volatile double vd3 = d3;
    volatile double vd4 = d4;
    
    /* Switch statement creates multiple basic blocks */
    switch (mode) {
        case 1: {
            /* More FP computations */
            double t0 = d15 + d16;
            double t1 = t0 * d17;
            double t2 = t1 + d18;
            
            /* Clobber FP/SSE registers */
            asm volatile("" : : : 
                "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                "xmm12", "xmm13", "xmm14", "xmm15"
            );
            
            /* Call at block end */
            bar(global_seed, d20);
            
            return vd0 + vd1 + vd2 + vd3 + vd4 + t0 + t1 + t2;
        }
        case 2:
            return d0 + d1;
        default:
            return d2 + d3;
    }
}

/* ========== Vector Register Pressure ========== */
#ifdef __SSE__
NOINLINE __m128 test_vector_pressure(int unroll) {
    /* Create vector register pressure */
    __m128 v0 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 v1 = _mm_add_ps(v0, v0);
    __m128 v2 = _mm_mul_ps(v1, v1);
    __m128 v3 = _mm_sub_ps(v2, v0);
    __m128 v4 = _mm_add_ps(v3, v1);
    __m128 v5 = _mm_mul_ps(v4, v2);
    __m128 v6 = _mm_sub_ps(v5, v3);
    __m128 v7 = _mm_add_ps(v6, v4);
    __m128 v8 = _mm_mul_ps(v7, v5);
    __m128 v9 = _mm_sub_ps(v8, v6);
    
    volatile __m128 vv0 = v0;
    volatile __m128 vv1 = v1;
    
    /* Loop with partial unrolling creates block ending with call */
    int sum = 0;
    for (int i = 0; i < unroll; i++) {
        if (i % 3 == 0) {
            /* More vector operations */
            __m128 t0 = _mm_add_ps(v9, v0);
            __m128 t1 = _mm_mul_ps(t0, v1);
            
            /* Clobber vector registers */
            asm volatile("" : : : 
                "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5",
                "ymm6", "ymm7", "ymm8", "ymm9", "ymm10", "ymm11",
                "ymm12", "ymm13", "ymm14", "ymm15"
            );
            
            /* Call at potential block end */
            baz(t0, _mm256_set_ps(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f));
            
            /* Use vectors after call */
            float f[4];
            _mm_store_ps(f, vv0);
            sum += (int)f[0];
        } else {
            sum += i;
        }
    }
    
    return _mm_set1_ps((float)sum);
}
#endif

/* ========== Mixed Pressure in Complex CFG ========== */
NOINLINE int test_mixed_pressure(int x) {
    /* Mixed integer and FP pressure */
    int i0 = x + 1;
    int i1 = i0 * 2;
    int i2 = i1 + x;
    int i3 = i2 ^ i1;
    int i4 = i3 << 2;
    
    double d0 = sin(x);
    double d1 = cos(x);
    double d2 = d0 + d1;
    double d3 = d1 * d2;
    
    volatile int vi0 = i0;
    volatile int vi1 = i1;
    volatile double vd0 = d0;
    volatile double vd1 = d1;
    
    /* Nested if-else creates complex CFG */
    if (x > 100) {
        if (x < 200) {
            int t0 = i4 + i2;
            double t1 = d3 * 2.0;
            
            /* Massive clobber list */
            asm volatile("" : : : 
                "rax", "rcx", "rdx", "rsi", "rdi",
                "r8", "r9", "r10", "r11",
                "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                "xmm6", "xmm7", "xmm8", "xmm9"
            );
            
            /* Call at block end after complex condition */
            foo();
            
            return vi0 + vi1 + (int)(vd0 + vd1 + t1) + t0;
        } else {
            return x;
        }
    } else if (x > 50) {
        return i0 + i1;
    } else {
        return (int)(d0 + d1);
    }
}

/* ========== Main Driver ========== */
int main(void) {
    int total = 0;
    
    /* Test all pressure scenarios */
    total += test_integer_pressure(global_seed);
    total += (int)test_fp_pressure(global_seed % 3);
    
    #ifdef __SSE__
    __m128 v = test_vector_pressure(6);
    float f[4];
    _mm_store_ps(f, v);
    total += (int)f[0];
    #endif
    
    total += test_mixed_pressure(global_seed);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
