/* test_caller_save.c - Forces GCC to insert save/restore instructions at call sites */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline, noipa))

/* External function that clobbers registers - defined in separate file */
NOINLINE void foo(void);
NOINLINE void bar(int, double);
NOINLINE void baz(__m128, __m128);

/* Helper to use variables after calls */
volatile int use_int;
volatile double use_double;
volatile __m128 use_vec;

/* Test 1: Integer register pressure with call at end of basic block */
NOINLINE int test_integer_pressure(int a, int b) {
    /* Create many integer live variables across a call */
    register int r0 = a + 1;
    volatile int v0 = r0 * 2;  /* Force memory access */
    register int r1 = v0 + b;
    register int r2 = r1 * 3;
    register int r3 = r2 - a;
    register int r4 = r3 + b;
    register int r5 = r4 * 2;
    register int r6 = r5 - r0;
    register int r7 = r6 + r1;
    register int r8 = r7 * r2;
    register int r9 = r8 - r3;
    register int r10 = r9 + r4;
    register int r11 = r10 * r5;
    register int r12 = r11 - r6;
    register int r13 = r12 + r7;
    register int r14 = r13 * r8;
    register int r15 = r14 - r9;
    register int r16 = r15 + r10;
    register int r17 = r16 * r11;
    register int r18 = r17 - r12;
    register int r19 = r18 + r13;
    register int r20 = r19 * r14;
    
    /* Clobber caller-saved registers explicitly */
    asm volatile("" : : : 
        "rax", "rcx", "rdx", "rsi", "rdi", 
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
        "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
        "xmm12", "xmm13", "xmm14", "xmm15");
    
    /* Call at potential block end - inside if/else */
    if (a > b) {
        /* More computations to increase pressure */
        r0 += r20;
        r1 += r19;
        r2 += r18;
        r3 += r17;
        
        /* Call with many live registers */
        foo();
        
        /* Use all variables after call */
        use_int = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 +
                 r10 + r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18 + r19 + r20;
        return use_int;
    } else {
        /* Different path to create separate basic block */
        return r20;
    }
}

/* Test 2: Floating-point pressure with complex control flow */
NOINLINE double test_fp_pressure(double x, double y) {
    /* Many FP variables */
    double d0 = sin(x);
    double d1 = cos(y);
    double d2 = d0 * d1;
    double d3 = d2 + x;
    double d4 = d3 * y;
    double d5 = sin(d4);
    double d6 = cos(d5);
    double d7 = d6 * d0;
    double d8 = d7 + d1;
    double d9 = d8 * d2;
    double d10 = sin(d9);
    double d11 = cos(d10);
    double d12 = d11 * d3;
    double d13 = d12 + d4;
    double d14 = d13 * d5;
    double d15 = sin(d14);
    double d16 = cos(d15);
    double d17 = d16 * d6;
    double d18 = d17 + d7;
    double d19 = d18 * d8;
    double d20 = sin(d19);
    
    /* Switch statement to create multiple basic blocks */
    int choice = (int)x % 4;
    double result = 0.0;
    
    switch (choice) {
        case 0:
            /* Call at end of this case's block */
            bar((int)d0, d1);
            result = d0 + d1 + d2;
            break;
        case 1:
            result = d3 + d4 + d5;
            bar((int)d3, d4);
            break;
        case 2:
            /* More computations before call */
            d20 = d20 * 2.0;
            d19 = d19 + 1.0;
            bar((int)d20, d19);
            result = d6 + d7 + d8;
            break;
        default:
            /* Call with many live FP values */
            for (int i = 0; i < 3; i++) {
                d0 += 0.1;
                d1 += 0.2;
                /* Call inside loop - creates block end at call */
                if (i == 1) {
                    bar((int)d0, d1);
                }
            }
            result = d9 + d10;
            break;
    }
    
    /* Use all FP variables */
    use_double = d0 + d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 +
                d10 + d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19 + d20;
    return result + use_double;
}

/* Test 3: Vector register pressure */
NOINLINE __m128 test_vector_pressure(float a, float b) {
    /* Many vector variables */
    __m128 v0 = _mm_set_ps(a, b, a+1, b+1);
    __m128 v1 = _mm_set_ps(b, a, b+2, a+2);
    __m128 v2 = _mm_add_ps(v0, v1);
    __m128 v3 = _mm_mul_ps(v0, v1);
    __m128 v4 = _mm_sub_ps(v2, v3);
    __m128 v5 = _mm_add_ps(v4, v0);
    __m128 v6 = _mm_mul_ps(v5, v1);
    __m128 v7 = _mm_sub_ps(v6, v2);
    __m128 v8 = _mm_add_ps(v7, v3);
    __m128 v9 = _mm_mul_ps(v8, v4);
    __m128 v10 = _mm_set_ps(a*2, b*2, a*3, b*3);
    __m128 v11 = _mm_add_ps(v9, v10);
    __m128 v12 = _mm_mul_ps(v11, v5);
    __m128 v13 = _mm_sub_ps(v12, v6);
    __m128 v14 = _mm_add_ps(v13, v7);
    __m128 v15 = _mm_mul_ps(v14, v8);
    
    /* Unrolled loop with call at end of iteration */
    __m128 sum = _mm_setzero_ps();
    for (int i = 0; i < 4; i++) {
        /* Partial unrolling */
        v0 = _mm_add_ps(v0, v1);
        v1 = _mm_add_ps(v1, v2);
        v2 = _mm_add_ps(v2, v3);
        
        /* Call that clobbers vector registers */
        if (i == 2) {
            /* This call should be at block end */
            baz(v0, v1);
        }
        
        v3 = _mm_add_ps(v3, v4);
        v4 = _mm_add_ps(v4, v5);
        
        if (i == 3) {
            baz(v2, v3);
        }
        
        sum = _mm_add_ps(sum, v0);
        sum = _mm_add_ps(sum, v1);
        sum = _mm_add_ps(sum, v2);
    }
    
    /* Use all vector variables */
    __m128 total = _mm_add_ps(v0, v1);
    total = _mm_add_ps(total, v2);
    total = _mm_add_ps(total, v3);
    total = _mm_add_ps(total, v4);
    total = _mm_add_ps(total, v5);
    total = _mm_add_ps(total, v6);
    total = _mm_add_ps(total, v7);
    total = _mm_add_ps(total, v8);
    total = _mm_add_ps(total, v9);
    total = _mm_add_ps(total, v10);
    total = _mm_add_ps(total, v11);
    total = _mm_add_ps(total, v12);
    total = _mm_add_ps(total, v13);
    total = _mm_add_ps(total, v14);
    total = _mm_add_ps(total, v15);
    
    use_vec = total;
    return _mm_add_ps(sum, total);
}

/* Test 4: Mixed pressure in nested control flow */
NOINLINE double test_mixed_pressure(int n, double x) {
    double result = x;
    
    /* Complex control flow with multiple blocks ending in calls */
    for (int i = 0; i < n; i++) {
        if (i % 3 == 0) {
            /* Integer pressure */
            int i0 = i + 1;
            int i1 = i0 * 2;
            int i2 = i1 + i;
            int i3 = i2 * 3;
            int i4 = i3 - i0;
            
            /* FP pressure */
            double d0 = sin(x + i);
            double d1 = cos(x - i);
            
            /* Call at end of this if block */
            bar(i0, d0);
            
            result += i0 + i1 + i2 + i3 + i4 + d0 + d1;
        } 
        else if (i % 3 == 1) {
            /* Different register pressure profile */
            double d2 = exp(x * i);
            double d3 = log(fabs(x) + 1.0);
            
            /* Vector pressure */
            __m128 v0 = _mm_set_ps(d2, d3, d2*2, d3*2);
            __m128 v1 = _mm_set_ps(i, i+1, i+2, i+3);
            
            /* Call with mixed live values */
            baz(v0, v1);
            
            float f[4];
            _mm_store_ps(f, v0);
            result += f[0] + f[1] + f[2] + f[3];
        }
        else {
            /* Simple call at block end */
            foo();
            result += 1.0;
        }
    }
    
    return result;
}

int main(void) {
    int total = 0;
    double sum = 0.0;
    
    /* Run all tests to create various pressure scenarios */
    total += test_integer_pressure(100, 50);
    sum += test_fp_pressure(1.57, 0.78);
    
    __m128 vec_result = test_vector_pressure(1.0f, 2.0f);
    float vec_sum[4];
    _mm_store_ps(vec_sum, vec_result);
    sum += vec_sum[0] + vec_sum[1] + vec_sum[2] + vec_sum[3];
    
    sum += test_mixed_pressure(10, 2.5);
    
    /* Ensure results are used */
    printf("Total: %d, Sum: %f\n", total, sum);
    
    return (total > 0 && sum > 0) ? 0 : 1;
}
