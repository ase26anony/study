/* test_caller_save.c - Forces GCC to insert save/restore instructions at call sites */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline, noipa))

/* External function that clobbers registers - defined in separate file */
NOINLINE void foo(void);
NOINLINE void bar(int, double);
NOINLINE void baz(__m128, __m128);

/* Helper to use variables and prevent optimization */
#define USE(var) asm volatile("" : "+r"(var))
#define USE_FP(var) asm volatile("" : "+x"(var))
#define USE_VEC(var) asm volatile("" : "+x"(var))

/* Test 1: Integer register pressure with call at block end */
NOINLINE int test_integer_pressure(int a, int b, int c) {
    volatile int v0 = a;  /* Force memory access */
    register int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9;
    register int r10, r11, r12, r13, r14, r15, r16, r17, r18, r19;
    
    /* Create long dependency chain to keep all registers live */
    r0 = v0 + 1; USE(r0);
    r1 = r0 * b + c; USE(r1);
    r2 = r1 - a * 2; USE(r2);
    r3 = r2 ^ r1; USE(r3);
    r4 = r3 * 3 + r0; USE(r4);
    r5 = r4 / (b + 1); USE(r5);
    r6 = r5 << 2; USE(r6);
    r7 = r6 | r3; USE(r7);
    r8 = r7 - r4; USE(r8);
    r9 = r8 * r5; USE(r9);
    r10 = r9 + r2; USE(r10);
    r11 = r10 ^ r8; USE(r11);
    r12 = r11 * 7; USE(r12);
    r13 = r12 - r6; USE(r13);
    r14 = r13 & r7; USE(r14);
    r15 = r14 + r9; USE(r15);
    r16 = r15 * 11; USE(r16);
    r17 = r16 / (r10 + 1); USE(r17);
    r18 = r17 << 1; USE(r18);
    r19 = r18 ^ r12; USE(r19);
    
    /* Call at the end of basic block (inside if) */
    if (a > 0) {
        /* Clobber caller-saved registers explicitly */
        asm volatile("" : : : 
            "rax", "rcx", "rdx", "rsi", "rdi", 
            "r8", "r9", "r10", "r11", 
            "xmm0", "xmm1", "xmm2", "xmm3",
            "xmm4", "xmm5", "xmm6", "xmm7",
            "xmm8", "xmm9", "xmm10", "xmm11",
            "xmm12", "xmm13", "xmm14", "xmm15");
        
        foo();  /* Call that clobbers registers */
        
        /* Use all registers after call - forces saves */
        int sum = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 +
                  r10 + r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18 + r19;
        return sum + v0;
    } else {
        /* Different path to create separate basic block */
        return a + b + c;
    }
}

/* Test 2: Floating-point pressure with switch statement */
NOINLINE double test_fp_pressure(double x, double y, int mode) {
    volatile double vx = x;  /* Force memory */
    double d0, d1, d2, d3, d4, d5, d6, d7, d8, d9;
    double d10, d11, d12, d13, d14, d15, d16, d17, d18, d19;
    
    /* Create FP computation chain */
    d0 = sin(vx); USE_FP(d0);
    d1 = cos(d0 * y); USE_FP(d1);
    d2 = d0 * d1 + x; USE_FP(d2);
    d3 = d2 / (y + 1.0); USE_FP(d3);
    d4 = sin(d3) * cos(d2); USE_FP(d4);
    d5 = d4 * d4 - d3; USE_FP(d5);
    d6 = exp(d5 * 0.1); USE_FP(d6);
    d7 = log(fabs(d6) + 1.0); USE_FP(d7);
    d8 = d7 * d5 + d4; USE_FP(d8);
    d9 = sin(d8) * cos(d7); USE_FP(d9);
    d10 = d9 + d6 * 2.0; USE_FP(d10);
    d11 = d10 / (d3 + 0.5); USE_FP(d11);
    d12 = sin(d11) + cos(d10); USE_FP(d12);
    d13 = d12 * d9 - d8; USE_FP(d13);
    d14 = exp(d13 * 0.2); USE_FP(d14);
    d15 = log(fabs(d14) + 2.0); USE_FP(d15);
    d16 = d15 * d12 + d11; USE_FP(d16);
    d17 = sin(d16) * 0.5; USE_FP(d17);
    d18 = cos(d17) * d14; USE_FP(d18);
    d19 = d18 + d15 * 3.0; USE_FP(d19);
    
    /* Switch creates multiple basic blocks */
    double result;
    switch (mode) {
        case 0:
            /* Call at end of this case's basic block */
            bar(42, x);
            result = d0 + d1 + d2 + d3 + d4 + d5;
            break;
        case 1:
            result = d6 + d7 + d8 + d9 + d10;
            bar(43, y);
            break;
        case 2:
            bar(44, d11);
            result = d11 + d12 + d13 + d14 + d15;
            break;
        case 3:  /* This case ends with call at block end */
            result = d16 + d17 + d18 + d19;
            /* More computations to increase pressure */
            d0 += sin(result);
            d1 += cos(result);
            bar(45, result);
            /* Call is at end of this basic block */
            break;
        default:
            result = x + y;
            break;
    }
    
    /* Use FP values across call */
    return result + d0 + d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 +
           d10 + d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19;
}

/* Test 3: Vector register pressure with loop unrolling */
NOINLINE __m128 test_vector_pressure(__m128 a, __m128 b) {
    __m128 v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    __m128 v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    
    /* Manual unrolling creates basic block ending with call */
    v0 = _mm_add_ps(a, b); USE_VEC(v0);
    v1 = _mm_mul_ps(v0, a); USE_VEC(v1);
    v2 = _mm_sub_ps(v1, b); USE_VEC(v2);
    v3 = _mm_add_ps(v2, v0); USE_VEC(v3);
    v4 = _mm_mul_ps(v3, v1); USE_VEC(v4);
    
    /* Partial loop to create control flow */
    for (int i = 0; i < 2; i++) {
        if (i == 0) {
            v5 = _mm_add_ps(v4, v2); USE_VEC(v5);
            v6 = _mm_mul_ps(v5, v3); USE_VEC(v6);
            v7 = _mm_sub_ps(v6, v4); USE_VEC(v7);
            
            /* Call in middle of loop body */
            baz(v0, v1);
            
            v8 = _mm_add_ps(v7, v5); USE_VEC(v8);
            v9 = _mm_mul_ps(v8, v6); USE_VEC(v9);
        } else {
            /* This block ends with call */
            v10 = _mm_add_ps(v9, v7); USE_VEC(v10);
            v11 = _mm_mul_ps(v10, v8); USE_VEC(v11);
            v12 = _mm_sub_ps(v11, v9); USE_VEC(v12);
            
            baz(v2, v3);  /* Call at end of basic block */
            
            /* Following code in different block */
            v13 = _mm_add_ps(v12, v10);
        }
    }
    
    v14 = _mm_add_ps(v13, v11); USE_VEC(v14);
    v15 = _mm_mul_ps(v14, v12); USE_VEC(v15);
    v16 = _mm_sub_ps(v15, v13); USE_VEC(v16);
    v17 = _mm_add_ps(v16, v14); USE_VEC(v17);
    v18 = _mm_mul_ps(v17, v15); USE_VEC(v18);
    v19 = _mm_sub_ps(v18, v16); USE_VEC(v19);
    
    /* Final call that might trigger save at block end */
    if (_mm_movemask_ps(v19) != 0) {
        __m128 temp = _mm_add_ps(v19, v17);
        baz(temp, v18);
        return _mm_add_ps(temp, v19);
    }
    
    return _mm_add_ps(v0, v1);
}

/* Test 4: Mixed pressure with complex control flow */
NOINLINE double test_mixed_pressure(int n, double x) {
    volatile int vi = n;
    volatile double vd = x;
    
    /* Integer pressure */
    register int i0 = vi + 1, i1 = i0 * 2, i2 = i1 + 3, i3 = i2 * 4;
    register int i4 = i3 - 5, i5 = i4 / 2, i6 = i5 << 1, i7 = i6 | 0xFF;
    register int i8 = i7 ^ i3, i9 = i8 * 3, i10 = i9 + i2, i11 = i10 - i5;
    
    /* FP pressure */
    double f0 = sin(vd), f1 = cos(f0), f2 = f0 * f1, f3 = f2 + x;
    double f4 = sin(f3), f5 = cos(f4), f6 = f4 * f5, f7 = f6 - f2;
    double f8 = exp(f7), f9 = log(fabs(f8) + 1.0), f10 = f8 * f9;
    
    /* Nested if-else to create basic blocks ending with calls */
    double result = 0.0;
    if (n > 100) {
        if (n > 200) {
            /* This block ends with call */
            bar(i0, f0);
            result = f0 + f1 + f2;
        } else {
            result = f3 + f4 + f5;
            bar(i1, f1);
        }
        
        /* Use all variables after call */
        result += i0 + i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10 + i11;
        result += f6 + f7 + f8 + f9 + f10;
    } else if (n > 50) {
        /* Different path */
        result = x * 2.0;
    } else {
        /* Call at end of this else block */
        for (int j = 0; j < 3; j++) {
            if (j == 2) {
                /* Call at end of innermost block */
                bar(i2, f2);
                /* BB_END should point to call before insertion */
            }
            result += j;
        }
    }
    
    return result;
}

/* Main function that calls all tests */
int main(void) {
    int int_result;
    double fp_result;
    __m128 vec_result;
    float vec_store[4];
    
    /* Call all test functions to trigger different pressure scenarios */
    int_result = test_integer_pressure(100, 200, 300);
    printf("Integer test result: %d\n", int_result);
    
    fp_result = test_fp_pressure(1.0, 2.0, 3);
    printf("FP test result: %f\n", fp_result);
    
    __m128 a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 b = _mm_set_ps(5.0f, 6.0f, 7.0f, 8.0f);
    vec_result = test_vector_pressure(a, b);
    _mm_storeu_ps(vec_store, vec_result);
    printf("Vector test result: %f %f %f %f\n", 
           vec_store[0], vec_store[1], vec_store[2], vec_store[3]);
    
    fp_result = test_mixed_pressure(150, 3.14159);
    printf("Mixed test result: %f\n", fp_result);
    
    return 0;
}
