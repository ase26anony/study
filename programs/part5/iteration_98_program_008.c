/* test_caller_save.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline))

/* External function that clobbers registers */
NOINLINE void foo(void);
NOINLINE void bar(int, double);
NOINLINE void baz(__m128, __m128);

/* Helper function in separate compilation unit */
void external_func(void);

/* Global volatile variables to prevent optimization */
volatile int global_counter = 0;
volatile double global_double = 3.14159;

/* Function 1: Integer register pressure with call at block end */
NOINLINE int test_integer_pressure(int param1, int param2) {
    /* Create many integer live variables across a call */
    register int r0 = param1 + 1;
    register int r1 = r0 * param2;
    volatile int v0 = r1 + 2;  /* Force memory access */
    register int r2 = v0 * 3;
    register int r3 = r2 - param1;
    register int r4 = r3 | 0xFF;
    register int r5 = r4 << 2;
    register int r6 = r5 ^ param2;
    register int r7 = r6 + global_counter;
    register int r8 = r7 * 2;
    register int r9 = r8 / 3;
    register int r10 = r9 & 0xFFFF;
    register int r11 = r10 + 1;
    register int r12 = r11 * 2;
    register int r13 = r12 - 3;
    register int r14 = r13 | 0xAA;
    register int r15 = r14 << 1;
    register int r16 = r15 ^ 0x55;
    register int r17 = r16 + 10;
    register int r18 = r17 * 2;
    register int r19 = r18 - 5;
    register int r20 = r19 | 0x33;
    
    /* Use inline assembly to clobber caller-saved registers */
    asm volatile("" : : : 
        "rax", "rcx", "rdx", "rsi", "rdi", 
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
        "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
        "xmm12", "xmm13", "xmm14", "xmm15");
    
    /* Call at potential block end - inside if/else */
    if (param1 > param2) {
        /* More computations to create basic block */
        register int t0 = r20 + param1;
        register int t1 = t0 * 2;
        volatile int v1 = t1 + global_counter;
        
        /* Non-inline call at what could be block end */
        foo();
        
        /* Use all variables after call */
        return r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + 
               r10 + r11 + r12 + r13 + r14 + r15 + r16 + r17 + 
               r18 + r19 + r20 + t0 + t1 + v0 + v1;
    } else {
        /* Alternative path */
        return r0 + r1 + r2;
    }
}

/* Function 2: Floating-point pressure with complex CFG */
NOINLINE double test_fp_pressure(double a, double b) {
    double d0 = sin(a) + 1.0;
    double d1 = cos(b) * d0;
    volatile double vd0 = d1 * global_double;
    double d2 = tan(a + b) + vd0;
    double d3 = exp(d2) * 2.0;
    double d4 = log(fabs(d3)) + 1.0;
    double d5 = pow(d4, 2.0);
    double d6 = sqrt(d5) + a;
    double d7 = d6 * 3.14159;
    double d8 = d7 / 2.71828;
    double d9 = sin(d8) * cos(d7);
    double d10 = d9 + d8 + d7 + d6;
    double d11 = d10 * 0.5;
    double d12 = d11 + d5 + d4;
    double d13 = d12 * d3;
    double d14 = d13 / d2;
    double d15 = d14 + d1 + d0;
    
    /* Create switch with multiple basic blocks */
    int choice = (int)a % 4;
    double result = 0.0;
    
    switch (choice) {
        case 0:
            /* Call in middle of case */
            bar((int)d0, d1);
            result = d0 + d1;
            break;
        case 1:
            /* Call at end of case block */
            {
                double temp = d2 + d3;
                volatile double vtemp = temp * 2.0;
                foo();  /* Potential block end */
                result = d2 + d3 + d4 + vtemp;
            }
            break;
        case 2:
            /* Multiple calls */
            bar((int)d5, d6);
            foo();
            result = d5 + d6 + d7;
            break;
        default:
            /* Loop with call at end */
            for (int i = 0; i < 3; i++) {
                double loop_var = d8 + i;
                volatile double vloop = loop_var * d9;
                if (i == 2) {
                    external_func();  /* Call at loop end */
                }
                result += loop_var + vloop;
            }
            break;
    }
    
    return result + d10 + d11 + d12 + d13 + d14 + d15;
}

/* Function 3: Vector/SIMD pressure */
#ifdef __SSE2__
NOINLINE __m128 test_vector_pressure(float f1, float f2, float f3, float f4) {
    __m128 v0 = _mm_set_ps(f1, f2, f3, f4);
    __m128 v1 = _mm_add_ps(v0, _mm_set1_ps(1.0f));
    __m128 v2 = _mm_mul_ps(v1, _mm_set1_ps(2.0f));
    __m128 v3 = _mm_sub_ps(v2, _mm_set1_ps(0.5f));
    __m128 v4 = _mm_add_ps(v3, v0);
    __m128 v5 = _mm_mul_ps(v4, v1);
    __m128 v6 = _mm_add_ps(v5, v2);
    __m128 v7 = _mm_sub_ps(v6, v3);
    __m128 v8 = _mm_mul_ps(v7, v4);
    __m128 v9 = _mm_add_ps(v8, v5);
    __m128 v10 = _mm_mul_ps(v9, _mm_set1_ps(3.0f));
    
    /* Mix with integer computations */
    int i0 = (int)f1;
    int i1 = i0 * 2;
    volatile int vi0 = i1 + 1;
    int i2 = vi0 * 3;
    int i3 = i2 | 0xFF;
    
    /* Call with vector arguments */
    baz(v0, v1);
    
    /* Use inline asm to clobber vector registers */
    asm volatile("" : : : 
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
        "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
        "xmm12", "xmm13", "xmm14", "xmm15",
        "rax", "rcx", "rdx");
    
    /* Conditional with call at end of then-block */
    if (f1 > f2) {
        __m128 v11 = _mm_add_ps(v10, v9);
        __m128 v12 = _mm_mul_ps(v11, v8);
        volatile float vf = f1 * f2;
        external_func();  /* Call at block end */
        return _mm_add_ps(v12, _mm_set1_ps(vf));
    } else {
        return _mm_add_ps(v10, v0);
    }
}
#endif

/* Function 4: Mixed pressure in loop */
NOINLINE double test_mixed_pressure_loop(int iterations) {
    double sum = 0.0;
    
    /* Unrolled loop to create multiple basic blocks */
    for (int i = 0; i < iterations; i += 2) {
        /* Integer computations */
        register int r0 = i + 1;
        register int r1 = r0 * 2;
        register int r2 = r1 + i;
        volatile int vr0 = r2 + global_counter;
        
        /* Floating computations */
        double d0 = sin(i * 0.1);
        double d1 = cos(i * 0.2);
        volatile double vd0 = d0 * d1;
        
        /* Call at end of loop iteration */
        if (i % 4 == 0) {
            bar(r0, d0);
        } else {
            foo();
        }
        
        /* Use all variables */
        sum += r0 + r1 + r2 + vr0 + d0 + d1 + vd0;
        
        /* Second part of unrolled iteration */
        if (i + 1 < iterations) {
            register int r3 = r2 + 1;
            register int r4 = r3 * 3;
            double d2 = exp(i * 0.3);
            volatile double vd1 = d2 * 2.0;
            
            /* Another call */
            external_func();
            
            sum += r3 + r4 + d2 + vd1;
        }
    }
    
    return sum;
}

/* Main function that calls all test cases */
int main(void) {
    int int_result = 0;
    double fp_result = 0.0;
    
    /* Test 1: Integer pressure */
    for (int i = 0; i < 10; i++) {
        int_result += test_integer_pressure(i, i * 2);
    }
    
    /* Test 2: FP pressure */
    for (double d = 0.1; d < 5.0; d += 0.5) {
        fp_result += test_fp_pressure(d, d * 2.0);
    }
    
    /* Test 3: Vector pressure */
    #ifdef __SSE2__
    float vec_result[4] = {0};
    for (float f = 0.1f; f < 2.0f; f += 0.3f) {
        __m128 v = test_vector_pressure(f, f*2, f*3, f*4);
        _mm_storeu_ps(vec_result, _mm_add_ps(_mm_loadu_ps(vec_result), v));
    }
    #endif
    
    /* Test 4: Mixed pressure in loop */
    double loop_result = test_mixed_pressure_loop(20);
    
    /* Use results to prevent dead code elimination */
    printf("Results: int=%d, fp=%f, loop=%f\n", 
           int_result, fp_result, loop_result);
    
    return 0;
}
