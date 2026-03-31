/* test_caller_save.c - Forces caller-save register spills at block ends */
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
NOINLINE double compute(double, double);
NOINLINE __m128 vector_op(__m128, __m128);

/* Helper functions in separate compilation unit */
extern void external_func1(void);
extern void external_func2(int);
extern double external_func3(double, double);

/* Global volatile variables to prevent optimization */
volatile int global_counter = 0;
volatile double global_accumulator = 0.0;

/* ========== Test Case 1: Integer register pressure at block end ========== */
NOINLINE int test_integer_pressure(int seed) {
    /* Create many integer live variables across a call */
    volatile int v0 = seed + 1;
    register int r1 = v0 * 2;
    register int r2 = r1 + seed;
    register int r3 = r2 ^ v0;
    register int r4 = r3 * 3;
    register int r5 = r4 - seed;
    register int r6 = r5 & 0xFF;
    register int r7 = r6 | 0x100;
    register int r8 = r7 << 2;
    register int r9 = r8 >> 1;
    register int r10 = r9 + v0;
    register int r11 = r10 * 5;
    register int r12 = r11 % 17;
    register int r13 = r12 ^ r1;
    register int r14 = r13 + r2;
    register int r15 = r14 * r3;
    register int r16 = r15 - r4;
    register int r17 = r16 & r5;
    register int r18 = r17 | r6;
    register int r19 = r18 << 3;
    register int r20 = r19 >> 2;
    
    /* Clobber many caller-saved registers with inline asm */
    asm volatile("" : : : 
        "rax", "rcx", "rdx", "rsi", "rdi", 
        "r8", "r9", "r10", "r11", "r12",
        "r13", "r14", "r15", "xmm0", "xmm1",
        "xmm2", "xmm3", "xmm4", "xmm5", "xmm6",
        "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
        "xmm12", "xmm13", "xmm14", "xmm15");
    
    /* Call at potential block end - inside if/else */
    if (seed % 3 == 0) {
        /* This creates a basic block ending with the call */
        foo();  /* Non-inlineable call */
        
        /* Use all the live variables after call */
        int result = r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 +
                    r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18 + r19 + r20;
        return result + v0;
    } else if (seed % 3 == 1) {
        /* Alternative path */
        external_func1();
        return r1 + r3 + r5 + r7 + r9;
    } else {
        /* Another alternative path */
        bar(r1, (double)r2);
        return r2 + r4 + r6 + r8 + r10;
    }
}

/* ========== Test Case 2: FP register pressure with complex CFG ========== */
NOINLINE double test_fp_pressure(double a, double b) {
    /* Many floating-point live variables */
    volatile double v0 = a + 1.0;
    double f1 = sin(v0);
    double f2 = cos(f1);
    double f3 = f1 * f2;
    double f4 = f3 + a;
    double f5 = f4 * b;
    double f6 = sin(f5);
    double f7 = cos(f6);
    double f8 = f7 * 2.0;
    double f9 = f8 - a;
    double f10 = sin(f9);
    double f11 = cos(f10);
    double f12 = f11 * 3.14159;
    double f13 = f12 / b;
    double f14 = sin(f13);
    double f15 = cos(f14);
    double f16 = f15 + f1;
    double f17 = f16 * f2;
    double f18 = f17 - f3;
    double f19 = f18 / f4;
    double f20 = f19 * f5;
    
    /* Switch creates multiple basic blocks */
    int choice = (int)a % 4;
    double result = 0.0;
    
    switch (choice) {
        case 0:
            /* Call at end of this case's block */
            external_func3(f1, f2);
            result = f1 + f3 + f5 + f7 + f9 + f11 + f13 + f15 + f17 + f19;
            break;
        case 1:
            foo();
            result = f2 + f4 + f6 + f8 + f10 + f12 + f14 + f16 + f18 + f20;
            break;
        case 2:
            /* Loop with call at end of unrolled iteration */
            for (int i = 0; i < 2; i++) {
                /* Partial unrolling creates block ending with call */
                if (i == 0) {
                    bar((int)f1, f2);
                } else {
                    external_func1();
                }
            }
            result = f1 * f20;
            break;
        default:
            compute(f1, f2);
            result = f20 - f1;
            break;
    }
    
    /* Use all FP variables after call */
    return result + f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 + f10 +
                   f11 + f12 + f13 + f14 + f15 + f16 + f17 + f18 + f19 + f20;
}

/* ========== Test Case 3: Vector register pressure ========== */
NOINLINE __m128 test_vector_pressure(__m128 vec_a, __m128 vec_b) {
    /* Many vector variables */
    __m128 v0 = _mm_add_ps(vec_a, vec_b);
    __m128 v1 = _mm_mul_ps(v0, vec_a);
    __m128 v2 = _mm_sub_ps(v1, vec_b);
    __m128 v3 = _mm_add_ps(v2, v0);
    __m128 v4 = _mm_mul_ps(v3, _mm_set1_ps(2.0f));
    __m128 v5 = _mm_sub_ps(v4, v1);
    __m128 v6 = _mm_add_ps(v5, v2);
    __m128 v7 = _mm_mul_ps(v6, v3);
    __m128 v8 = _mm_sub_ps(v7, v4);
    __m128 v9 = _mm_add_ps(v8, v5);
    __m128 v10 = _mm_mul_ps(v9, v6);
    __m128 v11 = _mm_sub_ps(v10, v7);
    __m128 v12 = _mm_add_ps(v11, v8);
    __m128 v13 = _mm_mul_ps(v12, v9);
    __m128 v14 = _mm_sub_ps(v13, v10);
    __m128 v15 = _mm_add_ps(v14, v11);
    
    /* Clobber vector registers */
    asm volatile("" : : : 
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
        "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
        "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7",
        "ymm8", "ymm9", "ymm10", "ymm11", "ymm12", "ymm13", "ymm14", "ymm15");
    
    /* Call in nested if structure */
    if (global_counter++ > 100) {
        external_func2(123);
    } else {
        if (((int*)&vec_a)[0] % 2 == 0) {
            /* This block ends with the call */
            vector_op(v0, v1);
        } else {
            foo();
        }
    }
    
    /* Use all vectors after call */
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
    result = _mm_add_ps(result, v15);
    
    return result;
}

/* ========== Test Case 4: Mixed register pressure in loop ========== */
NOINLINE double test_mixed_pressure(int iterations) {
    double acc = 0.0;
    volatile int vi = 0;
    
    /* Unrolled loop creates basic blocks ending with calls */
    for (int i = 0; i < iterations; i++) {
        /* Integer pressure */
        register int r1 = i * 2;
        register int r2 = r1 + 1;
        register int r3 = r2 ^ i;
        register int r4 = r3 * 3;
        
        /* FP pressure */
        double f1 = sin((double)i);
        double f2 = cos(f1);
        double f3 = f1 * f2;
        double f4 = f3 + acc;
        
        /* Vector pressure */
        __m128 vec1 = _mm_set1_ps((float)i);
        __m128 vec2 = _mm_set1_ps((float)r1);
        __m128 vec3 = _mm_add_ps(vec1, vec2);
        
        /* Call at end of loop body block */
        if (i % 3 == 0) {
            compute(f1, f2);
        } else if (i % 3 == 1) {
            bar(r1, f1);
        } else {
            external_func1();
        }
        
        /* Use variables after call */
        acc += f1 + f2 + f3 + f4 + (double)r1 + (double)r2 + (double)r3 + (double)r4;
        vi += ((int*)&vec3)[0];
    }
    
    return acc + vi;
}

/* ========== Main driver ========== */
int main(void) {
    int total = 0;
    double fp_total = 0.0;
    
    /* Run all test cases to trigger different spill scenarios */
    for (int i = 0; i < 10; i++) {
        total += test_integer_pressure(i);
        fp_total += test_fp_pressure((double)i, (double)i + 1.0);
        
        __m128 vec_a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
        __m128 vec_b = _mm_set_ps(5.0f, 6.0f, 7.0f, 8.0f);
        __m128 vec_result = test_vector_pressure(vec_a, vec_b);
        fp_total += ((float*)&vec_result)[0] + ((float*)&vec_result)[1] +
                   ((float*)&vec_result)[2] + ((float*)&vec_result)[3];
    }
    
    fp_total += test_mixed_pressure(5);
    
    /* Prevent dead code elimination */
    printf("Results: %d, %f\n", total, fp_total);
    return (total > 0 && fp_total > 0) ? 0 : 1;
}

/* ========== Dummy implementations ========== */
NOINLINE void foo(void) {
    global_accumulator += 0.1;
}

NOINLINE void bar(int x, double y) {
    global_accumulator += y + (double)x;
}

NOINLINE double compute(double a, double b) {
    return sin(a) * cos(b);
}

NOINLINE __m128 vector_op(__m128 a, __m128 b) {
    return _mm_add_ps(_mm_mul_ps(a, b), _mm_set1_ps(1.0f));
}
