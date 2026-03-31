/* test_caller_save.c - Forces caller-save register spilling at block ends */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline, noipa))

/* External function declarations */
NOINLINE void foo(void);
NOINLINE void bar(int, double);
NOINLINE int baz(void);

/* Helper functions in separate compilation unit */
extern void external_func1(void);
extern double external_func2(double);
extern void external_func3(__m128);

/* Global volatile variables to prevent optimization */
volatile int g_volatile_int = 42;
volatile double g_volatile_double = 3.14159;

/* ========== Test Case 1: Integer register pressure at block end ========== */
NOINLINE int test_integer_pressure(int cond) {
    /* Create massive integer register pressure */
    register int r0  = g_volatile_int + 1;
    register int r1  = r0 * 2;
    register int r2  = r1 + g_volatile_int;
    register int r3  = r2 - r0;
    register int r4  = r3 ^ r1;
    register int r5  = r4 | r2;
    register int r6  = r5 & r3;
    register int r7  = r6 << 2;
    register int r8  = r7 >> 1;
    register int r9  = r8 * r0;
    register int r10 = r9 / (r1 + 1);
    register int r11 = r10 % (r2 + 1);
    register int r12 = r11 + r3;
    register int r13 = r12 - r4;
    register int r14 = r13 * r5;
    register int r15 = r14 / (r6 + 1);
    register int r16 = r15 ^ r7;
    register int r17 = r16 | r8;
    register int r18 = r17 & r9;
    register int r19 = r18 << 3;
    register int r20 = r19 >> 2;
    
    /* Use inline assembly to clobber caller-saved integer registers */
    asm volatile("" : : : 
        "rax", "rcx", "rdx", "rsi", "rdi", 
        "r8", "r9", "r10", "r11", "r12", 
        "r13", "r14", "r15", "xmm0", "xmm1",
        "xmm2", "xmm3", "xmm4", "xmm5"
    );
    
    /* Complex control flow to create basic block ending with call */
    if (cond > 0) {
        /* This basic block ends with the call to foo() */
        int sum = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 +
                  r10 + r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18 + r19 + r20;
        
        /* Call at the end of basic block - forces potential BB_END update */
        foo();
        
        /* Use all variables after call to keep them live */
        return sum + r0 - r1 + r2 - r3 + r4 - r5 + r6 - r7 + r8 - r9 +
               r10 - r11 + r12 - r13 + r14 - r15 + r16 - r17 + r18 - r19 + r20;
    } else {
        /* Alternative path */
        return r0 + r1;
    }
}

/* ========== Test Case 2: FP register pressure with math functions ========== */
NOINLINE double test_fp_pressure(int iterations) {
    /* Create massive floating-point register pressure */
    volatile double v0 = g_volatile_double;
    double d0 = sin(v0);
    double d1 = cos(d0);
    double d2 = tan(d1);
    double d3 = exp(d2);
    double d4 = log(d3 + 1.0);
    double d5 = sqrt(d4);
    double d6 = pow(d5, 2.0);
    double d7 = fabs(d6 - 1.0);
    double d8 = sin(d7) * cos(d6);
    double d9 = tan(d8) + exp(d7);
    double d10 = log(d9) / sqrt(d8);
    double d11 = pow(d10, d9);
    double d12 = fmod(d11, 3.14159);
    double d13 = sin(d12) + cos(d11);
    double d14 = tan(d13) * exp(d12);
    double d15 = log(d14) - sqrt(d13);
    double d16 = pow(d15, 1.5);
    double d17 = fabs(d16);
    double d18 = sin(d17) * cos(d16);
    double d19 = tan(d18);
    double d20 = exp(d19);
    
    /* Clobber FP/SSE registers */
    asm volatile("" : : : 
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
        "xmm6", "xmm7", "xmm8", "xmm9", "xmm10",
        "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
    );
    
    /* Switch statement creates multiple basic blocks */
    double result = 0.0;
    switch (iterations % 4) {
        case 0:
            /* Call at end of this case's basic block */
            bar(1, d0);
            result = d0 + d1 + d2;
            break;
        case 1:
            result = d3 + d4 + d5;
            external_func2(d6);
            break;
        case 2:
            /* Multiple calls in different blocks */
            foo();
            result = d7 + d8 + d9 + d10;
            bar(2, d11);
            break;
        case 3:
        default:
            /* Call at end of default block */
            result = d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19 + d20;
            external_func2(result);
    }
    
    /* Use all variables to keep them live */
    return result + d0 + d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 +
           d10 + d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19 + d20;
}

/* ========== Test Case 3: Vector register pressure ========== */
#ifdef __SSE2__
NOINLINE __m128 test_vector_pressure(int mode) {
    /* Create vector register pressure */
    __m128 v0 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 v1 = _mm_add_ps(v0, v0);
    __m128 v2 = _mm_mul_ps(v1, v1);
    __m128 v3 = _mm_sub_ps(v2, v0);
    __m128 v4 = _mm_div_ps(v3, v1);
    __m128 v5 = _mm_sqrt_ps(v4);
    __m128 v6 = _mm_add_ps(v5, v0);
    __m128 v7 = _mm_mul_ps(v6, v2);
    __m128 v8 = _mm_sub_ps(v7, v3);
    __m128 v9 = _mm_div_ps(v8, v4);
    __m128 v10 = _mm_add_ps(v9, v5);
    __m128 v11 = _mm_mul_ps(v10, v6);
    __m128 v12 = _mm_sub_ps(v11, v7);
    __m128 v13 = _mm_div_ps(v12, v8);
    __m128 v14 = _mm_add_ps(v13, v9);
    __m128 v15 = _mm_mul_ps(v14, v10);
    
    /* Loop with call at end of unrolled iteration */
    __m128 accum = _mm_setzero_ps();
    for (int i = 0; i < 3; i++) {
        /* Partially unrolled loop body */
        __m128 temp1 = _mm_add_ps(v0, v1);
        __m128 temp2 = _mm_add_ps(v2, v3);
        __m128 temp3 = _mm_add_ps(v4, v5);
        
        /* Call at potential block end */
        if (i == 1) {
            external_func3(temp1);
        }
        
        accum = _mm_add_ps(accum, temp1);
        accum = _mm_add_ps(accum, temp2);
        accum = _mm_add_ps(accum, temp3);
        
        /* Another call in loop */
        if (mode == 0) {
            foo();
        }
    }
    
    return _mm_add_ps(accum, _mm_add_ps(
        _mm_add_ps(v0, v1),
        _mm_add_ps(_mm_add_ps(v2, v3), 
                  _mm_add_ps(_mm_add_ps(v4, v5),
                            _mm_add_ps(_mm_add_ps(v6, v7),
                                      _mm_add_ps(_mm_add_ps(v8, v9),
                                                _mm_add_ps(_mm_add_ps(v10, v11),
                                                          _mm_add_ps(_mm_add_ps(v12, v13),
                                                                    _mm_add_ps(v14, v15))))))));
}
#endif

/* ========== Test Case 4: Mixed pressure in nested loops ========== */
NOINLINE int test_mixed_pressure(void) {
    int i, j;
    volatile int vi[10];
    volatile double vd[10];
    
    /* Initialize volatile arrays */
    for (i = 0; i < 10; i++) {
        vi[i] = i + g_volatile_int;
        vd[i] = i * g_volatile_double;
    }
    
    int sum_int = 0;
    double sum_fp = 0.0;
    
    /* Nested loops with calls at block ends */
    for (i = 0; i < 5; i++) {
        /* Integer pressure in outer loop */
        register int r0 = vi[0] + i;
        register int r1 = vi[1] * r0;
        register int r2 = vi[2] + r1;
        register int r3 = vi[3] - r2;
        register int r4 = vi[4] ^ r3;
        
        for (j = 0; j < 3; j++) {
            /* FP pressure in inner loop */
            double d0 = sin(vd[0] + j);
            double d1 = cos(vd[1] * d0);
            double d2 = tan(vd[2] + d1);
            double d3 = exp(vd[3] - d2);
            
            /* Call at end of inner loop body (potential block end) */
            if (j == 1) {
                bar(r0, d0);
            }
            
            sum_fp += d0 + d1 + d2 + d3;
        }
        
        /* Call at end of outer loop iteration */
        if (i == 2 || i == 4) {
            foo();
        }
        
        sum_int += r0 + r1 + r2 + r3 + r4;
    }
    
    return sum_int + (int)sum_fp;
}

/* ========== Main function ========== */
int main(void) {
    int total = 0;
    
    /* Run all test cases to exercise different register pressure scenarios */
    total += test_integer_pressure(g_volatile_int);
    
    double fp_result = test_fp_pressure(g_volatile_int);
    total += (int)fp_result;
    
    #ifdef __SSE2__
    __m128 vec_result = test_vector_pressure(g_volatile_int % 2);
    float vec_floats[4];
    _mm_storeu_ps(vec_floats, vec_result);
    total += (int)(vec_floats[0] + vec_floats[1] + vec_floats[2] + vec_floats[3]);
    #endif
    
    total += test_mixed_pressure();
    
    /* Final call to ensure all paths are used */
    total += baz();
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}

/* ========== Helper functions (prevent inlining) ========== */
NOINLINE void foo(void) {
    /* Empty function that gets called */
    asm volatile("" : : : "memory");
}

NOINLINE void bar(int x, double y) {
    /* Function with arguments */
    g_volatile_int += x;
    g_volatile_double += y;
}

NOINLINE int baz(void) {
    return g_volatile_int;
}
