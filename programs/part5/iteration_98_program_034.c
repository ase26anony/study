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
NOINLINE int baz(void);

/* Helper functions in separate compilation unit */
extern void external_func1(void);
extern double external_func2(double);
extern void external_func3(__m128);

/* Global volatile to prevent optimization */
volatile int global_seed = 42;

/* Function 1: Integer register pressure with call at block end */
NOINLINE int test_integer_pressure(int cond) {
    /* Create many integer live variables across a call */
    volatile int v0 = global_seed;
    register int r1 asm ("r12") = v0 + 1;
    register int r2 asm ("r13") = r1 * 2;
    register int r3 asm ("r14") = r2 + v0;
    register int r4 asm ("r15") = r3 ^ r1;
    int r5 = r4 - 3;
    int r6 = r5 * 7;
    int r7 = r6 / 2;
    int r8 = r7 << 1;
    int r9 = r8 | 0xFF;
    int r10 = r9 & 0x0F;
    int r11 = r10 + 11;
    int r12 = r11 * r10;
    int r13 = r12 % 17;
    int r14 = r13 ^ r12;
    int r15 = r14 + r13;
    int r16 = r15 * 3;
    int r17 = r16 - 5;
    int r18 = r17 ^ 0xAAAA;
    int r19 = r18 + 19;
    int r20 = r19 * r18;
    int r21 = r20 / 7;
    int r22 = r21 << 2;
    int r23 = r22 | 0x5555;
    int r24 = r23 & 0x3333;
    int r25 = r24 + 25;
    int r26 = r25 * r24;
    int r27 = r26 % 13;
    int r28 = r27 ^ r26;
    int r29 = r28 + r27;
    int r30 = r29 * 11;
    
    /* Complex control flow to create basic block ending with call */
    if (cond > 0) {
        /* This basic block ends with the call to foo() */
        int sum = r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 +
                  r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18 + r19 + r20 +
                  r21 + r22 + r23 + r24 + r25 + r26 + r27 + r28 + r29 + r30;
        
        /* Inline assembly to clobber caller-saved registers */
        asm volatile("# Integer clobber" 
                     : 
                     : 
                     : "rax", "rcx", "rdx", "rsi", "rdi", 
                       "r8", "r9", "r10", "r11", 
                       "xmm0", "xmm1", "xmm2", "xmm3",
                       "xmm4", "xmm5", "xmm6", "xmm7",
                       "xmm8", "xmm9", "xmm10", "xmm11",
                       "xmm12", "xmm13", "xmm14", "xmm15");
        
        /* Call at end of basic block */
        foo();
        
        /* Use all variables after call to keep them live */
        return sum + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 +
               r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18 + r19 + r20 +
               r21 + r22 + r23 + r24 + r25 + r26 + r27 + r28 + r29 + r30;
    } else {
        /* Alternative path */
        return r30;
    }
}

/* Function 2: Floating-point register pressure */
NOINLINE double test_fp_pressure(int mode) {
    /* Many floating-point variables */
    volatile double d0 = (double)global_seed;
    double d1 = sin(d0);
    double d2 = cos(d1);
    double d3 = d1 + d2;
    double d4 = d3 * 1.5;
    double d5 = d4 / 2.0;
    double d6 = sqrt(fabs(d5));
    double d7 = d6 * M_PI;
    double d8 = exp(d7);
    double d9 = log(fabs(d8) + 1.0);
    double d10 = d9 * d8;
    double d11 = d10 - d9;
    double d12 = pow(d11, 2.0);
    double d13 = d12 + d11;
    double d14 = sin(d13) * cos(d13);
    double d15 = d14 * 3.14159;
    double d16 = d15 / 2.71828;
    double d17 = d16 + d15;
    double d18 = d17 * d16;
    double d19 = d18 - d17;
    double d20 = sqrt(d19 * d19 + 1.0);
    
    /* Switch to create multiple basic blocks */
    double result;
    switch (mode) {
        case 0:
            /* Call at end of this case's basic block */
            asm volatile("# FP clobber" 
                         : 
                         : 
                         : "xmm0", "xmm1", "xmm2", "xmm3",
                           "xmm4", "xmm5", "xmm6", "xmm7",
                           "xmm8", "xmm9", "xmm10", "xmm11",
                           "xmm12", "xmm13", "xmm14", "xmm15");
            
            external_func2(d1);
            result = d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10 +
                     d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19 + d20;
            break;
            
        case 1:
            result = d20;
            break;
            
        default:
            result = d1 * d20;
            break;
    }
    
    return result + d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10 +
           d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19 + d20;
}

/* Function 3: Vector register pressure */
NOINLINE __m128 test_vector_pressure(int iter) {
    /* Many vector variables */
    __m128 v0 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 v1 = _mm_add_ps(v0, v0);
    __m128 v2 = _mm_mul_ps(v1, _mm_set1_ps(2.0f));
    __m128 v3 = _mm_sub_ps(v2, v1);
    __m128 v4 = _mm_div_ps(v3, _mm_set1_ps(1.5f));
    __m128 v5 = _mm_add_ps(v4, v3);
    __m128 v6 = _mm_mul_ps(v5, v4);
    __m128 v7 = _mm_sub_ps(v6, v5);
    __m128 v8 = _mm_add_ps(v7, v6);
    __m128 v9 = _mm_mul_ps(v8, _mm_set1_ps(0.5f));
    __m128 v10 = _mm_add_ps(v9, v8);
    __m128 v11 = _mm_mul_ps(v10, v9);
    __m128 v12 = _mm_sub_ps(v11, v10);
    __m128 v13 = _mm_add_ps(v12, v11);
    __m128 v14 = _mm_mul_ps(v13, _mm_set1_ps(1.1f));
    __m128 v15 = _mm_add_ps(v14, v13);
    
    /* Loop with partial unrolling - call at end of unrolled block */
    __m128 accum = _mm_setzero_ps();
    for (int i = 0; i < iter; i++) {
        if (i % 3 == 0) {
            /* This block ends with a call */
            asm volatile("# Vector clobber"
                         :
                         :
                         : "xmm0", "xmm1", "xmm2", "xmm3",
                           "xmm4", "xmm5", "xmm6", "xmm7",
                           "xmm8", "xmm9", "xmm10", "xmm11",
                           "xmm12", "xmm13", "xmm14", "xmm15",
                           "ymm0", "ymm1", "ymm2", "ymm3",
                           "ymm4", "ymm5", "ymm6", "ymm7",
                           "ymm8", "ymm9", "ymm10", "ymm11",
                           "ymm12", "ymm13", "ymm14", "ymm15");
            
            external_func3(v0);
            accum = _mm_add_ps(accum, v0);
        } else if (i % 3 == 1) {
            accum = _mm_add_ps(accum, v1);
        } else {
            accum = _mm_add_ps(accum, v2);
        }
        
        /* Rotate vectors to keep them all live */
        __m128 temp = v0;
        v0 = v1; v1 = v2; v2 = v3; v3 = v4; v4 = v5; v5 = v6;
        v6 = v7; v7 = v8; v8 = v9; v9 = v10; v10 = v11; v11 = v12;
        v12 = v13; v13 = v14; v14 = v15; v15 = temp;
    }
    
    return _mm_add_ps(accum, _mm_add_ps(v0, _mm_add_ps(v1, _mm_add_ps(v2, 
               _mm_add_ps(v3, _mm_add_ps(v4, _mm_add_ps(v5, _mm_add_ps(v6,
               _mm_add_ps(v7, _mm_add_ps(v8, _mm_add_ps(v9, _mm_add_ps(v10,
               _mm_add_ps(v11, _mm_add_ps(v12, _mm_add_ps(v13, 
               _mm_add_ps(v14, v15)))))))))))))));
}

/* Function 4: Mixed register pressure with nested calls */
NOINLINE double test_mixed_pressure(int x, int y) {
    /* Integer pressure */
    volatile int vi0 = x;
    int vi1 = vi0 + y;
    int vi2 = vi1 * 2;
    int vi3 = vi2 - vi1;
    int vi4 = vi3 ^ vi2;
    int vi5 = vi4 * 3;
    int vi6 = vi5 / 2;
    int vi7 = vi6 << 1;
    int vi8 = vi7 | 0xFF;
    int vi9 = vi8 & 0x0F;
    
    /* Floating-point pressure */
    double vd0 = (double)x;
    double vd1 = sin(vd0);
    double vd2 = cos(vd1);
    double vd3 = vd1 + vd2;
    double vd4 = vd3 * 1.5;
    double vd5 = vd4 / 2.0;
    
    /* Complex control flow with call at merge point */
    double result;
    if (x > 0) {
        if (y > 0) {
            /* Call at end of this inner block */
            asm volatile("# Mixed clobber"
                         :
                         :
                         : "rax", "rcx", "rdx", "rsi", "rdi",
                           "r8", "r9", "r10", "r11",
                           "xmm0", "xmm1", "xmm2", "xmm3",
                           "xmm4", "xmm5", "xmm6", "xmm7");
            
            bar(vi1, vd1);
            result = (double)vi1 + vd1;
        } else {
            result = (double)vi2 + vd2;
        }
    } else {
        if (y > 0) {
            result = (double)vi3 + vd3;
        } else {
            result = (double)vi4 + vd4;
        }
    }
    
    /* Use all variables after potential calls */
    return result + (double)(vi0 + vi1 + vi2 + vi3 + vi4 + vi5 + 
                             vi6 + vi7 + vi8 + vi9) +
                    vd0 + vd1 + vd2 + vd3 + vd4 + vd5;
}

/* Main function that exercises all test cases */
int main(void) {
    int total = 0;
    double fp_total = 0.0;
    
    /* Test 1: Integer pressure with various conditions */
    for (int i = -5; i <= 5; i++) {
        total += test_integer_pressure(i);
    }
    
    /* Test 2: FP pressure with different modes */
    for (int mode = 0; mode < 3; mode++) {
        fp_total += test_fp_pressure(mode);
    }
    
    /* Test 3: Vector pressure */
    __m128 vec_result = test_vector_pressure(10);
    float vec_floats[4];
    _mm_store_ps(vec_floats, vec_result);
    fp_total += vec_floats[0] + vec_floats[1] + vec_floats[2] + vec_floats[3];
    
    /* Test 4: Mixed pressure */
    for (int x = -2; x <= 2; x++) {
        for (int y = -2; y <= 2; y++) {
            fp_total += test_mixed_pressure(x, y);
        }
    }
    
    /* Use results to prevent dead code elimination */
    printf("Integer total: %d\n", total);
    printf("FP total: %f\n", fp_total);
    
    return (total > 0 && fp_total != 0.0) ? 0 : 1;
}

/* Dummy functions to satisfy references */
void foo(void) {
    global_seed++;
}

void bar(int a, double b) {
    global_seed += a + (int)b;
}

int baz(void) {
    return global_seed;
}
