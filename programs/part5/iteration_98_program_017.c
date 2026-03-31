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
volatile int use_result;

/* Test 1: Integer register pressure with call at block end */
NOINLINE int test_integer_pressure(int a, int b, int c) {
    /* Create many integer live values across a call */
    volatile int v0 = a;
    register int r1 = b + 1;
    register int r2 = r1 * 2;
    register int r3 = r2 + c;
    register int r4 = r3 - a;
    register int r5 = r4 * 3;
    register int r6 = r5 / 2;
    register int r7 = r6 ^ b;
    register int r8 = r7 | c;
    register int r9 = r8 & 0xFF;
    register int r10 = r9 << 2;
    register int r11 = r10 >> 1;
    register int r12 = r11 + r1;
    register int r13 = r12 - r2;
    register int r14 = r13 * r3;
    register int r15 = r14 / 4;
    register int r16 = r15 ^ r4;
    register int r17 = r16 | r5;
    register int r18 = r17 & r6;
    register int r19 = r18 + r7;
    register int r20 = r19 - r8;
    
    /* Use control flow to create basic block ending with call */
    int result;
    if (a > 0) {
        /* This block ends with the call to foo() */
        foo();  /* Non-inline call */
        
        /* Use all register values after call */
        result = r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 +
                 r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18 + r19 + r20;
    } else {
        /* Alternative path - different register usage */
        result = a + b + c;
    }
    
    /* Force use of volatile */
    use_result = v0;
    
    return result;
}

/* Test 2: Floating-point pressure with complex CFG */
NOINLINE double test_fp_pressure(double x, double y) {
    /* Many FP live values */
    volatile double vd0 = x;
    double d1 = sin(x);
    double d2 = cos(y);
    double d3 = d1 * d2;
    double d4 = d3 + x;
    double d5 = d4 - y;
    double d6 = sin(d5);
    double d7 = cos(d6);
    double d8 = d7 * d1;
    double d9 = d8 / d2;
    double d10 = exp(d9);
    double d11 = log(fabs(d10) + 1.0);
    double d12 = d11 * d3;
    double d13 = d12 + d4;
    double d14 = d13 - d5;
    double d15 = sin(d14);
    double d16 = cos(d15);
    double d17 = d16 * d6;
    double d18 = d17 / d7;
    double d19 = exp(d18);
    double d20 = log(fabs(d19) + 1.0);
    
    /* Create switch with multiple basic blocks */
    double result;
    switch ((int)x % 4) {
        case 0:
            /* Block ending with call */
            bar((int)x, y);
            result = d1 + d2 + d3 + d4 + d5;
            break;
        case 1:
            result = d6 + d7 + d8 + d9 + d10;
            break;
        case 2:
            /* Another block ending with call */
            bar((int)y, x);
            result = d11 + d12 + d13 + d14 + d15;
            break;
        default:
            result = d16 + d17 + d18 + d19 + d20;
            break;
    }
    
    use_result = (int)vd0;
    return result;
}

/* Test 3: Vector register pressure with inline assembly clobbers */
#ifdef __SSE__
NOINLINE __m128 test_vector_pressure(__m128 a, __m128 b) {
    /* Many vector live values */
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
    
    /* Inline assembly that clobbers many registers */
    asm volatile(""
        : 
        : 
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", 
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15",
          "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
    
    /* Loop with call at end of basic block */
    __m128 result = v0;
    for (int i = 0; i < 3; i++) {
        if (i == 1) {
            /* This block ends with call */
            float tmp[4];
            _mm_store_ps(tmp, v9);
            baz(v8, _mm256_set1_ps(tmp[0]));
        }
        result = _mm_add_ps(result, v1);
        v1 = _mm_add_ps(v1, v2);
    }
    
    return _mm_add_ps(result, v9);
}
#endif

/* Test 4: Mixed pressure in nested loops */
NOINLINE int test_mixed_pressure(int iter) {
    int sum = 0;
    double acc = 0.0;
    
    /* Unrolled loop creates multiple basic blocks */
    for (int i = 0; i < iter; i++) {
        /* Integer pressure */
        register int t0 = i * 2;
        register int t1 = t0 + 1;
        register int t2 = t1 * 3;
        register int t3 = t2 - i;
        register int t4 = t3 ^ 0x55;
        
        /* FP pressure */
        double f0 = sin(i * 0.1);
        double f1 = cos(i * 0.2);
        double f2 = f0 * f1;
        double f3 = f2 + acc;
        
        /* Call at end of loop body basic block */
        if (i % 2 == 0) {
            bar(t0, f0);
        } else {
            foo();
        }
        
        /* Use values after call */
        sum += t0 + t1 + t2 + t3 + t4;
        acc = f3;
    }
    
    return sum + (int)acc;
}

/* Test 5: Large switch with calls at block ends */
NOINLINE int test_switch_pressure(int key) {
    volatile int v = key;
    int result = 0;
    
    /* Many register-tied variables */
    register int r0 = key * 1;
    register int r1 = key * 2;
    register int r2 = key * 3;
    register int r3 = key * 4;
    register int r4 = key * 5;
    register int r5 = key * 6;
    register int r6 = key * 7;
    register int r7 = key * 8;
    register int r8 = key * 9;
    register int r9 = key * 10;
    
    switch (key & 0x7) {
        case 0:
            foo();
            result = r0 + r1;
            break;
        case 1:
            bar(r2, 1.0);
            result = r2 + r3;
            break;
        case 2:
            foo();
            result = r4 + r5;
            break;
        case 3:
            bar(r6, 2.0);
            result = r6 + r7;
            break;
        case 4:
            foo();
            result = r8 + r9;
            break;
        case 5:
            /* Multiple calls in same block */
            foo();
            bar(r0, 3.0);
            result = r0 + r9;
            break;
        case 6:
            result = r1 + r8;
            break;
        default:
            foo();
            result = r0;
            break;
    }
    
    use_result = v;
    return result;
}

int main(void) {
    int total = 0;
    
    /* Run all tests to trigger different pressure scenarios */
    total += test_integer_pressure(1, 2, 3);
    total += test_integer_pressure(-1, 5, 7);
    
    total += (int)test_fp_pressure(1.0, 2.0);
    total += (int)test_fp_pressure(3.0, 4.0);
    
    #ifdef __SSE__
    __m128 va = _mm_set1_ps(1.0f);
    __m128 vb = _mm_set1_ps(2.0f);
    __m128 vc = test_vector_pressure(va, vb);
    float vf[4];
    _mm_store_ps(vf, vc);
    total += (int)vf[0];
    #endif
    
    total += test_mixed_pressure(10);
    total += test_switch_pressure(42);
    
    printf("Result: %d\n", total);
    return 0;
}
