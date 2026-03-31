/* test_caller_save.c - Forces GCC to insert save/restore instructions at call sites */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR volatile

/* External function that clobbers registers - defined in separate file */
NOINLINE void foo(void);
NOINLINE void bar(int, double);
NOINLINE void baz(__m128, __m128);

/* Helper to use variables and prevent optimization */
NOINLINE int use_int(int x) { VOLATILE_VAR int y = x; return y + 1; }
NOINLINE double use_double(double x) { VOLATILE_VAR double y = x; return y * 1.1; }

/* Test 1: Integer register pressure with call at end of basic block */
NOINLINE int test_integer_pressure(int seed) {
    VOLATILE_VAR int a = seed;
    
    /* Create many integer live variables that must survive across call */
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
    register int r12 = r11 * r6;
    register int r13 = r12 - r7;
    register int r14 = r13 | r8;
    register int r15 = r14 ^ r9;
    register int r16 = r15 & r10;
    register int r17 = r16 + r11;
    register int r18 = r17 * r12;
    register int r19 = r18 - r13;
    register int r20 = r19 | r14;
    
    /* Use inline assembly to clobber caller-saved integer registers */
    asm volatile("" : : : 
        "rax", "rcx", "rdx", "rsi", "rdi", 
        "r8", "r9", "r10", "r11", "r12", 
        "r13", "r14", "r15", "xmm0", "xmm1",
        "xmm2", "xmm3", "xmm4", "xmm5", "xmm6",
        "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
        "xmm12", "xmm13", "xmm14", "xmm15");
    
    /* Complex control flow to create basic block ending with call */
    if (a > 100) {
        /* This creates a basic block ending with foo() call */
        foo();  /* Call at end of basic block */
        
        /* Use all variables after call to keep them live */
        return r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 +
               r10 + r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18 + r19 + r20;
    } else {
        /* Alternative path */
        return a * 2;
    }
}

/* Test 2: Floating-point register pressure */
NOINLINE double test_float_pressure(double seed) {
    VOLATILE_VAR double d = seed;
    
    /* Many floating-point computations */
    double f0 = sin(d);
    double f1 = cos(d);
    double f2 = f0 * f1;
    double f3 = f2 + d;
    double f4 = exp(f3);
    double f5 = log(f4);
    double f6 = f5 * f0;
    double f7 = f6 / f1;
    double f8 = sin(f7);
    double f9 = cos(f8);
    double f10 = f9 * f2;
    double f11 = f10 + f3;
    double f12 = exp(f11);
    double f13 = log(f12);
    double f14 = f13 * f4;
    double f15 = f14 / f5;
    double f16 = sin(f15);
    double f17 = cos(f16);
    double f18 = f17 * f6;
    double f19 = f18 + f7;
    double f20 = exp(f19);
    
    /* Clobber FP registers */
    asm volatile("" : : : 
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
        "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
        "rax", "rcx", "rdx");
    
    /* Switch statement creates multiple basic blocks */
    switch ((int)d % 4) {
        case 0:
            bar(1, f0);  /* Call at end of basic block */
            break;
        case 1:
            bar(2, f1);
            break;
        case 2:
            bar(3, f2);
            break;
        default:
            bar(4, f3);
            break;
    }
    
    /* Use all FP variables */
    return f0 + f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 +
           f10 + f11 + f12 + f13 + f14 + f15 + f16 + f17 + f18 + f19 + f20;
}

/* Test 3: Vector register pressure */
NOINLINE __m128 test_vector_pressure(float seed) {
    __m128 v0 = _mm_set1_ps(seed);
    __m128 v1 = _mm_add_ps(v0, _mm_set1_ps(1.0f));
    __m128 v2 = _mm_mul_ps(v1, _mm_set1_ps(2.0f));
    __m128 v3 = _mm_sub_ps(v2, v0);
    __m128 v4 = _mm_div_ps(v3, v1);
    __m128 v5 = _mm_add_ps(v4, v2);
    __m128 v6 = _mm_mul_ps(v5, v3);
    __m128 v7 = _mm_sub_ps(v6, v4);
    __m128 v8 = _mm_add_ps(v7, v5);
    __m128 v9 = _mm_mul_ps(v8, v6);
    __m128 v10 = _mm_set1_ps(seed * 0.5f);
    __m128 v11 = _mm_add_ps(v10, v7);
    __m128 v12 = _mm_mul_ps(v11, v8);
    __m128 v13 = _mm_sub_ps(v12, v9);
    __m128 v14 = _mm_add_ps(v13, v10);
    
    /* Loop with call at end of unrolled iteration */
    for (int i = 0; i < 3; i++) {
        if (i == 1) {
            /* Call at end of basic block created by loop unrolling */
            baz(v0, v1);
        }
        
        /* Vector operations that keep variables live */
        v0 = _mm_add_ps(v0, v1);
        v1 = _mm_mul_ps(v1, v2);
    }
    
    /* Return combination of all vectors */
    __m128 result = _mm_add_ps(v0, v1);
    result = _mm_add_ps(result, v2);
    result = _mm_add_ps(result, v3);
    result = _mm_add_ps(result, v4);
    result = _mm_add_ps(result, v5);
    result = _mm_add_ps(result, v6);
    result = _mm_add_ps(result, v7);
    result = _mm_add_ps(result, v8);
    result = _mm_add_ps(result, v9);
    result = _mm_add_ps(result, v10);
    result = _mm_add_ps(result, v11);
    result = _mm_add_ps(result, v12);
    result = _mm_add_ps(result, v13);
    result = _mm_add_ps(result, v14);
    
    return result;
}

/* Test 4: Mixed register pressure with complex CFG */
NOINLINE double test_mixed_pressure(int i_seed, double d_seed) {
    VOLATILE_VAR int a = i_seed;
    VOLATILE_VAR double d = d_seed;
    
    /* Integer variables */
    int i0 = a * 2;
    int i1 = i0 + 1;
    int i2 = i1 * a;
    int i3 = i2 - i0;
    int i4 = i3 | i1;
    int i5 = i4 ^ i2;
    
    /* Floating-point variables */
    double f0 = sin(d);
    double f1 = cos(d);
    double f2 = f0 * f1;
    double f3 = exp(f2);
    double f4 = log(f3);
    
    /* Nested control flow */
    if (a > 0) {
        if (d > 0.5) {
            /* Call at end of inner basic block */
            foo();
            
            /* More computations after call */
            i0 = i5 + 1;
            f0 = f4 * 2.0;
        } else {
            bar(i0, f0);
        }
        
        /* Use variables in loop */
        for (int j = 0; j < 2; j++) {
            i1 += j;
            f1 += 0.1;
            
            if (j == 0) {
                /* Another call site */
                asm volatile("" : : : 
                    "rax", "rbx", "rcx", "rdx", 
                    "xmm0", "xmm1", "xmm2", "xmm3");
                foo();
            }
        }
    } else {
        baz(_mm_set1_ps((float)d), _mm_set1_ps((float)f0));
    }
    
    /* Ensure all variables are used */
    return (double)(i0 + i1 + i2 + i3 + i4 + i5) + f0 + f1 + f2 + f3 + f4;
}

int main(void) {
    int int_result = 0;
    double float_result = 0.0;
    __m128 vec_result;
    float vec_sum[4] = {0};
    
    /* Call all test functions with different parameters */
    for (int i = 0; i < 10; i++) {
        int_result += test_integer_pressure(i * 50);
        float_result += test_float_pressure((double)i * 0.5);
        
        vec_result = test_vector_pressure((float)i * 0.25f);
        _mm_storeu_ps(vec_sum, _mm_add_ps(_mm_loadu_ps(vec_sum), vec_result));
        
        float_result += test_mixed_pressure(i, (double)i * 0.3);
    }
    
    /* Use results to prevent dead code elimination */
    printf("Results: int=%d, float=%f, vector=[%f,%f,%f,%f]\n",
           int_result, float_result,
           vec_sum[0], vec_sum[1], vec_sum[2], vec_sum[3]);
    
    return 0;
}
