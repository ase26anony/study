/* test_caller_save.c - Forces GCC to insert save/restore instructions at call sites */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline, noclone))

/* External function that clobbers registers - defined in separate file */
NOINLINE void foo(void);
NOINLINE void bar(int, double);
NOINLINE void baz(__m128, __m128);

/* Helper to use variables after calls */
volatile int use_result;

/* ========== Test 1: Integer register pressure at block end ========== */
NOINLINE int test_integer_pressure(int seed) {
    volatile int v0 = seed;  /* Force memory access */
    
    /* Create many integer live variables across a call */
    register int r0  = v0 + 1;
    register int r1  = r0 * 2;
    register int r2  = r1 + v0;
    register int r3  = r2 ^ r1;
    register int r4  = r3 << 2;
    register int r5  = r4 - r3;
    register int r6  = r5 | 0xFF;
    register int r7  = r6 & 0x0F;
    register int r8  = r7 * 3;
    register int r9  = r8 / 2;
    register int r10 = r9 + 100;
    register int r11 = r10 - 50;
    register int r12 = r11 * r10;
    register int r13 = r12 ^ r11;
    register int r14 = r13 << 1;
    register int r15 = r14 >> 2;
    register int r16 = r15 + r14;
    register int r17 = r16 * 7;
    register int r18 = r17 % 13;
    register int r19 = r18 | r17;
    register int r20 = r19 & 0xFFFF;
    
    /* Clobber many caller-saved registers explicitly */
    asm volatile("" : : : 
        "rax", "rcx", "rdx", "rsi", "rdi", 
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
        "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
        "xmm12", "xmm13", "xmm14", "xmm15");
    
    /* Create basic block structure where call is at end */
    if (seed > 0) {
        /* This block ends with the call to foo() */
        foo();  /* Non-inline call at block end */
        
        /* Use all variables after call to keep them live */
        use_result = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 +
                    r10 + r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18 + r19 + r20;
        return use_result;
    } else {
        /* Alternative path - creates separate basic block */
        return -1;
    }
}

/* ========== Test 2: Floating-point pressure with math calls ========== */
NOINLINE double test_fp_pressure(double seed) {
    volatile double v = seed;
    
    /* Many FP variables that must survive across call */
    double d0 = sin(v);
    double d1 = cos(v);
    double d2 = d0 * d1;
    double d3 = d2 + v;
    double d4 = d3 * 2.0;
    double d5 = d4 / 3.14159;
    double d6 = exp(d5);
    double d7 = log(fabs(d6) + 1.0);
    double d8 = d7 * d6;
    double d9 = d8 - d7;
    double d10 = d9 * d8;
    double d11 = d10 + d9;
    double d12 = d11 * 0.5;
    double d13 = d12 * d11;
    double d14 = d13 / (d12 + 1.0);
    double d15 = d14 * d13;
    double d16 = d15 + 14.0;
    double d17 = d16 * 1.1;
    double d18 = d17 - 0.1;
    double d19 = d18 * d17;
    double d20 = d19 / 2.0;
    
    /* Call with arguments - uses more registers */
    bar((int)seed, d0);
    
    /* Use results after call */
    return d0 + d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 +
           d10 + d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19 + d20;
}

/* ========== Test 3: Vector/SIMD register pressure ========== */
#ifdef __SSE2__
NOINLINE __m128 test_vector_pressure(float seed) {
    /* Create many vector variables */
    __m128 v0 = _mm_set1_ps(seed);
    __m128 v1 = _mm_add_ps(v0, _mm_set1_ps(1.0f));
    __m128 v2 = _mm_mul_ps(v1, _mm_set1_ps(2.0f));
    __m128 v3 = _mm_sub_ps(v2, v1);
    __m128 v4 = _mm_div_ps(v3, _mm_set1_ps(3.0f));
    __m128 v5 = _mm_add_ps(v4, v3);
    __m128 v6 = _mm_mul_ps(v5, v4);
    __m128 v7 = _mm_sub_ps(v6, v5);
    __m128 v8 = _mm_set_ps(8.0f, 7.0f, 6.0f, 5.0f);
    __m128 v9 = _mm_add_ps(v8, v7);
    __m128 v10 = _mm_mul_ps(v9, v8);
    
    /* Switch creates multiple basic blocks */
    switch ((int)seed) {
        case 0:
            /* Call at end of this case's basic block */
            baz(v0, v1);
            /* Fall through */
        case 1:
            /* Use vectors after call */
            v0 = _mm_add_ps(v0, v10);
            break;
        case 2:
            baz(v2, v3);
            v0 = _mm_add_ps(v0, v9);
            break;
        default:
            baz(v4, v5);
            v0 = _mm_add_ps(v0, v8);
            break;
    }
    
    return _mm_add_ps(v0, _mm_add_ps(v1, _mm_add_ps(v2, v3)));
}
#endif

/* ========== Test 4: Mixed pressure in loop ========== */
NOINLINE double test_mixed_pressure(int iterations) {
    double total = 0.0;
    volatile int vi = 0;
    
    /* Unrolled loop creates multiple basic blocks with calls */
    for (int i = 0; i < iterations; i++) {
        /* Integer pressure */
        register int r0 = i + vi;
        register int r1 = r0 * 2;
        register int r2 = r1 + i;
        
        /* FP pressure */
        double d0 = sin(i * 0.1);
        double d1 = cos(i * 0.2);
        
        /* Call at potential block end */
        if (i & 1) {
            foo();
            total += d0 + r0;
        } else {
            bar(r1, d1);
            total += d1 + r1;
        }
        
        /* Use variables after call */
        vi += r2;
    }
    
    return total + vi;
}

/* ========== Test 5: Nested calls with pressure ========== */
NOINLINE int test_nested_pressure(int level, int seed) {
    if (level <= 0) {
        /* Base case - create register pressure before returning */
        int r0 = seed * 2;
        int r1 = r0 + seed;
        int r2 = r1 * r0;
        foo();  /* Call at end of block before return */
        return r0 + r1 + r2;
    }
    
    /* Recursive call with live variables across it */
    int a = seed * level;
    int b = a + 1;
    int c = b * 2;
    
    int result = test_nested_pressure(level - 1, seed + 1);
    
    /* Variables must survive across recursive call */
    return result + a + b + c;
}

/* ========== Main driver ========== */
int main(void) {
    double total = 0.0;
    
    /* Run all tests to trigger different pressure scenarios */
    total += test_integer_pressure(42);
    total += test_fp_pressure(3.14159);
    
    #ifdef __SSE2__
    __m128 v = test_vector_pressure(1.0f);
    float f[4];
    _mm_storeu_ps(f, v);
    total += f[0] + f[1] + f[2] + f[3];
    #endif
    
    total += test_mixed_pressure(10);
    total += test_nested_pressure(3, 5);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %f\n", total);
    
    return (total > 0) ? 0 : 1;
}
