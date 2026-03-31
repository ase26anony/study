/* test_caller_save.c - Forces GCC to insert save/restore instructions at call sites */

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline))

/* External function that clobbers registers - defined in separate file */
NOINLINE void foo(void);
NOINLINE void bar(int, double);
NOINLINE void baz(__m128, __m256);

/* Helper to use variables after calls */
volatile int sink;

/* Test 1: Integer register pressure with call at block end */
NOINLINE int test_integer_pressure(int a, int b, int c) {
    /* Create many integer live variables across a call */
    volatile int v0 = a;
    register int r1 = b + 1;
    register int r2 = r1 * 2;
    register int r3 = r2 + c;
    register int r4 = r3 - a;
    register int r5 = r4 * 3;
    register int r6 = r5 / 2;
    register int r7 = r6 ^ b;
    register int r8 = r7 | c;
    register int r9 = r8 & a;
    register int r10 = r9 << 2;
    register int r11 = r10 >> 1;
    register int r12 = r11 + v0;
    register int r13 = r12 * r1;
    register int r14 = r13 - r2;
    register int r15 = r14 + r3;
    register int r16 = r15 ^ r4;
    register int r17 = r16 | r5;
    register int r18 = r17 & r6;
    register int r19 = r18 + r7;
    register int r20 = r19 * r8;
    
    /* Use inline assembly to clobber caller-saved integer registers */
    asm volatile("" : : : 
        "rax", "rcx", "rdx", "rsi", "rdi", 
        "r8", "r9", "r10", "r11", "r12", 
        "r13", "r14", "r15", "xmm0", "xmm1",
        "xmm2", "xmm3", "xmm4", "xmm5"
    );
    
    /* Call at potential block end - inside if/else */
    int result;
    if (a > b) {
        /* This creates a basic block ending with foo() */
        foo();
        
        /* Use all the register variables after call */
        result = r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 +
                 r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18 + r19 + r20;
    } else {
        /* Alternative path */
        result = a + b + c;
    }
    
    /* Force use of variables */
    sink = result;
    return result;
}

/* Test 2: Floating-point pressure with complex control flow */
NOINLINE double test_fp_pressure(double x, double y) {
    volatile double vd0 = x;
    double d1 = sin(x);
    double d2 = cos(y);
    double d3 = d1 * d2;
    double d4 = d3 + x;
    double d5 = d4 * y;
    double d6 = sin(d5);
    double d7 = cos(d6);
    double d8 = d7 * d1;
    double d9 = d8 + d2;
    double d10 = sin(d9);
    double d11 = cos(d10);
    double d12 = d11 * d3;
    double d13 = d12 + d4;
    double d14 = sin(d13);
    double d15 = cos(d14);
    double d16 = d15 * d5;
    double d17 = d16 + d6;
    double d18 = sin(d17);
    double d19 = cos(d18);
    double d20 = d19 * d7;
    
    /* Clobber FP registers */
    asm volatile("" : : : 
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4",
        "xmm5", "xmm6", "xmm7", "xmm8", "xmm9",
        "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
    );
    
    /* Call inside switch at block end */
    double result;
    switch ((int)x % 4) {
        case 0:
            bar((int)x, y);
            /* Use FP variables after call */
            result = d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10 +
                     d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19 + d20;
            break;
        case 1:
            result = d1 * d2;
            break;
        case 2:
            result = d3 - d4;
            break;
        default:
            result = vd0;
            break;
    }
    
    sink = (int)result;
    return result;
}

#ifdef __SSE__
/* Test 3: Vector register pressure */
NOINLINE __m128 test_vector_pressure(__m128 a, __m128 b) {
    /* Create many vector variables */
    __m128 v0 = _mm_add_ps(a, b);
    __m128 v1 = _mm_mul_ps(v0, a);
    __m128 v2 = _mm_sub_ps(v1, b);
    __m128 v3 = _mm_add_ps(v2, v0);
    __m128 v4 = _mm_mul_ps(v3, v1);
    __m128 v5 = _mm_sub_ps(v4, v2);
    __m128 v6 = _mm_add_ps(v5, v3);
    __m128 v7 = _mm_mul_ps(v6, v4);
    __m128 v8 = _mm_sub_ps(v7, v5);
    __m128 v9 = _mm_add_ps(v8, v6);
    __m128 v10 = _mm_mul_ps(v9, v7);
    
    /* Clobber vector registers */
    asm volatile("" : : : 
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4",
        "xmm5", "xmm6", "xmm7", "xmm8", "xmm9",
        "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
    );
    
    /* Call in loop with partial unrolling */
    __m128 result = v0;
    for (int i = 0; i < 3; i++) {
        if (i == 1) {
            /* This creates a block ending with baz() */
            baz(v1, _mm256_set1_ps(1.0f));
            /* Use vectors after call */
            result = _mm_add_ps(result, v2);
            result = _mm_add_ps(result, v3);
        }
        result = _mm_add_ps(result, v4);
    }
    
    return result;
}
#endif

/* Test 4: Mixed pressure with call at tail of multiple blocks */
NOINLINE int test_mixed_pressure(int a, double b) {
    volatile int vi = a;
    volatile double vd = b;
    
    /* Integer pressure */
    register int i1 = a * 2;
    register int i2 = i1 + 1;
    register int i3 = i2 * 3;
    register int i4 = i3 - a;
    
    /* FP pressure */
    double d1 = sin(b);
    double d2 = cos(b);
    double d3 = d1 * d2;
    
    /* Clobber mixed registers */
    asm volatile("" : : : 
        "rax", "rcx", "rdx", "rsi", "rdi",
        "r8", "r9", "r10", "r11",
        "xmm0", "xmm1", "xmm2", "xmm3",
        "xmm4", "xmm5", "xmm6", "xmm7"
    );
    
    int result;
    
    /* Nested control flow to create complex CFG */
    if (a > 0) {
        if (b > 0.5) {
            /* Call at end of this block */
            foo();
            result = i1 + i2 + (int)(d1 * 100);
        } else {
            bar(a, b);
            result = i3 + i4 + (int)(d2 * 100);
        }
        
        /* More code after - prevents tail merging */
        result += vi;
    } else {
        switch (a % 3) {
            case 0:
                foo();
                result = (int)(d3 * 100);
                break;
            case 1:
                /* Another call at block end */
                bar(a, b);
                result = i1 * 2;
                break;
            default:
                result = a;
                break;
        }
    }
    
    /* Force use of all variables */
    sink = result + (int)vd;
    return result;
}

/* Main driver that calls all tests */
int main(void) {
    int total = 0;
    
    /* Test integer pressure */
    total += test_integer_pressure(1, 2, 3);
    total += test_integer_pressure(10, 20, 30);
    
    /* Test FP pressure */
    total += (int)test_fp_pressure(1.0, 2.0);
    total += (int)test_fp_pressure(3.14, 2.71);
    
#ifdef __SSE__
    /* Test vector pressure */
    __m128 vec_a = _mm_set1_ps(1.0f);
    __m128 vec_b = _mm_set1_ps(2.0f);
    __m128 vec_result = test_vector_pressure(vec_a, vec_b);
    float f[4];
    _mm_store_ps(f, vec_result);
    total += (int)f[0];
#endif
    
    /* Test mixed pressure */
    total += test_mixed_pressure(5, 1.5);
    total += test_mixed_pressure(-3, 0.25);
    
    printf("Total: %d\n", total);
    return 0;
}
