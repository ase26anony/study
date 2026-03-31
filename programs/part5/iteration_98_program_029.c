/* test_caller_save.c - Forces GCC to insert save/restore instructions at call sites */

#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline, noipa))

/* External function that clobbers registers - defined in separate file */
NOINLINE void foo(void);
NOINLINE void bar(int, double);
NOINLINE int baz(void);

/* Helper to use variables to prevent optimization */
#define USE(var) asm volatile("" : : "r"(var))

/* Test 1: Integer register pressure with call at end of basic block */
NOINLINE int test_integer_pressure(int seed) {
    volatile int trigger = seed; /* Force memory access */
    
    /* Create many integer live variables across a call */
    register int r0  = trigger + 1;
    register int r1  = r0 * 2;
    register int r2  = r1 + trigger;
    register int r3  = r2 - trigger;
    register int r4  = r3 * 3;
    register int r5  = r4 / 2;
    register int r6  = r5 ^ trigger;
    register int r7  = r6 | 0xFF;
    register int r8  = r7 & 0x0F;
    register int r9  = r8 << 2;
    register int r10 = r9 >> 1;
    register int r11 = r10 + 100;
    register int r12 = r11 - 50;
    register int r13 = r12 * 2;
    register int r14 = r13 / 3;
    register int r15 = r14 ^ 0xAAAA;
    register int r16 = r15 | 0x5555;
    register int r17 = r16 & 0x3333;
    register int r18 = r17 << 1;
    register int r19 = r18 >> 2;
    register int r20 = r19 + r0;
    
    /* Use inline assembly to clobber caller-saved registers */
    asm volatile("" : : : 
        "rax", "rcx", "rdx", "rsi", "rdi", 
        "r8", "r9", "r10", "r11", 
        "xmm0", "xmm1", "xmm2", "xmm3",
        "xmm4", "xmm5", "xmm6", "xmm7"
    );
    
    /* Call at potential end of basic block */
    if (trigger > 0) {
        /* This creates a basic block ending with foo() */
        foo();  /* Non-inline call */
        
        /* Use all variables after call - must be preserved */
        return r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 +
               r10 + r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18 + r19 + r20;
    } else {
        /* Alternative path to create CFG */
        return trigger;
    }
}

/* Test 2: Floating-point pressure with complex control flow */
NOINLINE double test_fp_pressure(double seed) {
    volatile double vseed = seed;
    
    /* Many FP variables that must survive across call */
    double d0 = sin(vseed);
    double d1 = cos(vseed);
    double d2 = d0 * d1;
    double d3 = d2 + vseed;
    double d4 = d3 * 2.0;
    double d5 = d4 / 3.14159;
    double d6 = exp(d5);
    double d7 = log(fabs(d6) + 1.0);
    double d8 = d7 * d0;
    double d9 = d8 - d1;
    double d10 = d9 * d2;
    double d11 = d10 + d3;
    double d12 = d11 * d4;
    double d13 = d12 / d5;
    double d14 = d13 + d6;
    double d15 = d14 - d7;
    double d16 = d15 * d8;
    double d17 = d16 / d9;
    double d18 = d17 + d10;
    double d19 = d18 * d11;
    double d20 = d19 - d12;
    
    /* Switch creates multiple basic blocks */
    int choice = (int)vseed % 3;
    double result = 0.0;
    
    switch (choice) {
        case 0:
            /* Call at end of this case's basic block */
            foo();
            result = d0 + d1 + d2;
            break;
        case 1:
            /* More computations then call */
            d0 = d0 * 2.0;
            d1 = d1 + 1.0;
            bar((int)vseed, d0);  /* Another non-inline call */
            result = d3 + d4 + d5;
            break;
        case 2:
            /* Loop with call at end of iteration */
            for (int i = 0; i < 2; i++) {
                d0 = d0 * 1.1;
                /* Call at potential end of loop body block */
                if (i == 1) {
                    foo();
                }
            }
            result = d6 + d7 + d8;
            break;
    }
    
    /* Use all FP variables after control flow */
    return result + d9 + d10 + d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19 + d20;
}

/* Test 3: Vector/SIMD register pressure */
#ifdef __SSE2__
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
    
    /* Clobber vector registers */
    asm volatile("" : : : 
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
        "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
    );
    
    /* Nested if-else to create interesting CFG */
    if (seed > 0.5f) {
        if (seed < 0.75f) {
            foo();  /* Call at end of inner block */
        } else {
            bar(1, (double)seed);
        }
        /* Use vectors after call */
        v0 = _mm_add_ps(v0, v1);
    } else {
        bar(0, (double)seed);
    }
    
    /* Force use of all vector variables */
    __m128 sum = _mm_add_ps(v0, v1);
    sum = _mm_add_ps(sum, v2);
    sum = _mm_add_ps(sum, v3);
    sum = _mm_add_ps(sum, v4);
    sum = _mm_add_ps(sum, v5);
    sum = _mm_add_ps(sum, v6);
    sum = _mm_add_ps(sum, v7);
    sum = _mm_add_ps(sum, v8);
    sum = _mm_add_ps(sum, v9);
    sum = _mm_add_ps(sum, v10);
    
    return sum;
}
#endif

/* Test 4: Mixed register pressure with loop */
NOINLINE double test_mixed_pressure(int iterations) {
    double acc = 0.0;
    volatile int vi = 0;
    
    /* Unrolled loop to create basic blocks ending with calls */
    for (int i = 0; i < iterations; i++) {
        /* Integer pressure */
        int i0 = i * 2;
        int i1 = i0 + 1;
        int i2 = i1 * 3;
        int i3 = i2 - i;
        int i4 = i3 / 2;
        
        /* FP pressure */
        double d0 = sin((double)i);
        double d1 = cos(d0);
        double d2 = d0 * d1;
        
        /* Call at end of loop body (potential block end) */
        if (i % 2 == 0) {
            foo();
        } else {
            bar(i, d0);
        }
        
        /* Use variables after call */
        acc += i0 + i1 + i2 + i3 + i4 + d0 + d1 + d2;
        
        /* Additional call in middle of block */
        if (i == iterations / 2) {
            vi = baz();
        }
    }
    
    return acc;
}

/* Test 5: Large switch with calls at case ends */
NOINLINE int test_switch_pressure(int val) {
    int result = 0;
    
    /* Many live variables across switch */
    register int a = val * 2;
    register int b = a + 1;
    register int c = b * 3;
    register int d = c - val;
    register int e = d / 2;
    register int f = e ^ 0xFF;
    
    switch (val % 5) {
        case 0:
            foo();
            result = a + b;  /* Call at end of this case block */
            break;
        case 1:
            bar(a, (double)b);
            result = c + d;
            break;
        case 2:
            /* Multiple calls in same case */
            foo();
            bar(b, (double)c);
            result = e + f;
            break;
        case 3:
            /* Call then computation */
            foo();
            a = a * 2;
            b = b + 1;
            result = a + b + c;
            break;
        case 4:
            /* Nested control flow */
            if (a > 0) {
                foo();  /* Call at end of if block */
                result = d + e;
            } else {
                bar(c, (double)d);
                result = e + f;
            }
            break;
    }
    
    /* Use all variables after switch */
    return result + a + b + c + d + e + f;
}

int main(void) {
    int total = 0;
    double ftotal = 0.0;
    
    /* Run all tests to exercise different pressure scenarios */
    total += test_integer_pressure(42);
    ftotal += test_fp_pressure(3.14159);
    
    #ifdef __SSE2__
    {
        __m128 v = test_vector_pressure(0.7f);
        float f[4];
        _mm_store_ps(f, v);
        ftotal += f[0] + f[1] + f[2] + f[3];
    }
    #endif
    
    ftotal += test_mixed_pressure(10);
    total += test_switch_pressure(7);
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d, %f\n", total, ftotal);
    
    return 0;
}
