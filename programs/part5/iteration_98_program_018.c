/* test_caller_save.c - Forces GCC to insert save/restore instructions at call sites */

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline))

/* External function that clobbers registers */
NOINLINE void foo(void);
NOINLINE void bar(int, double);
NOINLINE void baz(__m128);

/* Helper functions in separate compilation unit */
void external_func1(void);
void external_func2(int);
double external_func3(double);

/* Global volatile to prevent optimizations */
volatile int global_counter = 0;

/* ========== Integer Register Pressure ========== */
NOINLINE int test_integer_pressure(int seed) {
    /* Create massive integer register pressure */
    volatile int v0 = seed + 1;
    register int r1 = v0 * 2;
    register int r2 = r1 + seed;
    register int r3 = r2 * 3;
    register int r4 = r3 - seed;
    register int r5 = r4 / 2;
    register int r6 = r5 ^ seed;
    register int r7 = r6 << 2;
    register int r8 = r7 >> 1;
    register int r9 = r8 | 0xFF;
    register int r10 = r9 & 0xAA;
    register int r11 = r10 + r1;
    register int r12 = r11 * r2;
    register int r13 = r12 - r3;
    register int r14 = r13 ^ r4;
    register int r15 = r14 | r5;
    register int r16 = r15 & r6;
    register int r17 = r16 + r7;
    register int r18 = r17 * r8;
    register int r19 = r18 - r9;
    register int r20 = r19 ^ r10;
    
    /* More variables to exhaust registers */
    int t1 = r20 + 1;
    int t2 = t1 * 2;
    int t3 = t2 + r1;
    int t4 = t3 * r2;
    int t5 = t4 - r3;
    int t6 = t5 ^ r4;
    int t7 = t6 | r5;
    int t8 = t7 & r6;
    int t9 = t8 + r7;
    int t10 = t9 * r8;
    
    /* Inline assembly to clobber caller-saved integer registers */
    asm volatile("" : : : 
        "rax", "rcx", "rdx", "rsi", "rdi", 
        "r8", "r9", "r10", "r11", "r12", 
        "r13", "r14", "r15", "xmm0", "xmm1",
        "xmm2", "xmm3", "xmm4", "xmm5"
    );
    
    /* Call at potential block end - inside if/else */
    if (seed % 2) {
        /* This creates a basic block ending with the call */
        foo();  /* Non-inline call */
        
        /* Use all the variables after call */
        return v0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 +
               r10 + r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18 + r19 +
               r20 + t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
    } else {
        /* Alternative path */
        bar(seed, (double)seed);
        return seed * 2;
    }
}

/* ========== Floating Point Register Pressure ========== */
NOINLINE double test_fp_pressure(double seed) {
    /* Massive FP register pressure */
    volatile double v0 = seed + 1.0;
    double d1 = sin(v0);
    double d2 = cos(d1);
    double d3 = d1 * d2;
    double d4 = d3 + seed;
    double d5 = sin(d4);
    double d6 = cos(d5);
    double d7 = d5 * d6;
    double d8 = d7 - seed;
    double d9 = sin(d8);
    double d10 = cos(d9);
    double d11 = d9 * d10;
    double d12 = d11 / 2.0;
    double d13 = sin(d12);
    double d14 = cos(d13);
    double d15 = d13 + d14;
    double d16 = d15 * 3.14159;
    double d17 = sin(d16);
    double d18 = cos(d17);
    double d19 = d17 * d18;
    double d20 = d19 + 1.618;
    
    /* More FP variables */
    double f1 = d20 * 2.0;
    double f2 = f1 + d1;
    double f3 = f2 * d2;
    double f4 = f3 - d3;
    double f5 = sin(f4);
    double f6 = cos(f5);
    double f7 = f5 * f6;
    double f8 = f7 + d4;
    double f9 = sin(f8);
    double f10 = cos(f9);
    
    /* Clobber FP/vector registers */
    asm volatile("" : : : 
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
        "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
        "xmm12", "xmm13", "xmm14", "xmm15"
    );
    
    /* Call at end of switch case block */
    switch ((int)seed % 4) {
        case 0:
            external_func3(d1);
            /* Fall through */
        case 1:
            /* This block ends with call */
            foo();
            break;
        case 2:
            bar((int)seed, d2);
            break;
        default:
            external_func3(d3);
            break;
    }
    
    /* Use all FP values after call */
    return v0 + d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 +
           d10 + d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19 +
           d20 + f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 + f10;
}

/* ========== Vector Register Pressure ========== */
#ifdef __SSE__
NOINLINE __m128 test_vector_pressure(__m128 seed) {
    /* Create vector register pressure */
    __m128 v1 = _mm_add_ps(seed, _mm_set1_ps(1.0f));
    __m128 v2 = _mm_mul_ps(v1, _mm_set1_ps(2.0f));
    __m128 v3 = _mm_sub_ps(v2, seed);
    __m128 v4 = _mm_add_ps(v3, v1);
    __m128 v5 = _mm_mul_ps(v4, v2);
    __m128 v6 = _mm_sub_ps(v5, v3);
    __m128 v7 = _mm_add_ps(v6, v4);
    __m128 v8 = _mm_mul_ps(v7, v5);
    __m128 v9 = _mm_sub_ps(v8, v6);
    __m128 v10 = _mm_add_ps(v9, v7);
    
    /* More vector variables */
    __m128 w1 = _mm_mul_ps(v10, _mm_set1_ps(0.5f));
    __m128 w2 = _mm_add_ps(w1, v1);
    __m128 w3 = _mm_mul_ps(w2, v2);
    __m128 w4 = _mm_sub_ps(w3, v3);
    __m128 w5 = _mm_add_ps(w4, v4);
    __m128 w6 = _mm_mul_ps(w5, v5);
    __m128 w7 = _mm_sub_ps(w6, v6);
    __m128 w8 = _mm_add_ps(w7, v7);
    __m128 w9 = _mm_mul_ps(w8, v8);
    __m128 w10 = _mm_sub_ps(w9, v9);
    
    /* Clobber vector registers */
    asm volatile("" : : : 
        "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5",
        "ymm6", "ymm7", "ymm8", "ymm9", "ymm10", "ymm11",
        "ymm12", "ymm13", "ymm14", "ymm15"
    );
    
    /* Call inside loop with partial unrolling */
    for (int i = 0; i < 3; i++) {
        if (i == 1) {
            /* This creates a block ending with call */
            baz(v1);
        } else {
            external_func2(i);
        }
    }
    
    /* Use vectors after call */
    __m128 result = _mm_add_ps(v1, v2);
    result = _mm_add_ps(result, v3);
    result = _mm_add_ps(result, v4);
    result = _mm_add_ps(result, v5);
    result = _mm_add_ps(result, v6);
    result = _mm_add_ps(result, v7);
    result = _mm_add_ps(result, v8);
    result = _mm_add_ps(result, v9);
    result = _mm_add_ps(result, v10);
    result = _mm_add_ps(result, w1);
    result = _mm_add_ps(result, w2);
    result = _mm_add_ps(result, w3);
    result = _mm_add_ps(result, w4);
    result = _mm_add_ps(result, w5);
    result = _mm_add_ps(result, w6);
    result = _mm_add_ps(result, w7);
    result = _mm_add_ps(result, w8);
    result = _mm_add_ps(result, w9);
    result = _mm_add_ps(result, w10);
    
    return result;
}
#endif

/* ========== Mixed Pressure in Complex CFG ========== */
NOINLINE int test_complex_cfg(int mode) {
    int result = 0;
    
    /* Large switch with register pressure in each case */
    switch (mode % 5) {
        case 0: {
            /* Integer pressure with call at block end */
            volatile int a = mode;
            register int b = a * 2;
            register int c = b + 1;
            register int d = c * 3;
            register int e = d - a;
            register int f = e / 2;
            register int g = f ^ 0x55;
            register int h = g << 1;
            register int i = h >> 2;
            register int j = i | 0xFF;
            
            asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi");
            
            /* Call at the end of this basic block */
            foo();
            
            result = a + b + c + d + e + f + g + h + i + j;
            break;
        }
        
        case 1: {
            /* FP pressure with call in middle */
            double x = (double)mode;
            double y = sin(x);
            double z = cos(y);
            
            external_func3(x);
            
            double a = y * z;
            double b = sin(a);
            double c = cos(b);
            
            result = (int)(a + b + c);
            break;
        }
        
        case 2: {
            /* Loop with call at end of iteration */
            for (int k = 0; k < 4; k++) {
                volatile int v = mode + k;
                register int r1 = v * 2;
                register int r2 = r1 + k;
                
                if (k == 2) {
                    /* Call at end of this block */
                    bar(v, (double)r1);
                } else {
                    external_func2(v);
                }
                
                result += r1 + r2;
            }
            break;
        }
        
        case 3: {
            /* Nested if-else with calls */
            volatile int base = mode;
            if (base % 3 == 0) {
                register int x = base * 3;
                register int y = x + 1;
                foo();
                result = x + y;
            } else if (base % 3 == 1) {
                register int x = base * 5;
                bar(x, (double)x);
                result = x * 2;
            } else {
                register int x = base * 7;
                external_func2(x);
                result = x / 2;
            }
            break;
        }
        
        default: {
            /* Default case with mixed pressure */
            volatile int v = mode;
            double d = (double)v;
            register int r1 = v * 11;
            register int r2 = r1 + 13;
            double d1 = sin(d);
            double d2 = cos(d1);
            
            asm volatile("" : : : "rax", "rdx", "xmm0", "xmm1", "xmm2");
            
            /* Multiple calls in sequence */
            external_func1();
            foo();
            external_func3(d1);
            
            result = r1 + r2 + (int)d1 + (int)d2;
            break;
        }
    }
    
    return result;
}

/* ========== Main Function ========== */
int main(void) {
    int total = 0;
    
    /* Test integer pressure */
    for (int i = 0; i < 10; i++) {
        total += test_integer_pressure(i + global_counter);
    }
    
    /* Test FP pressure */
    for (int i = 0; i < 5; i++) {
        total += (int)test_fp_pressure((double)i + 1.0);
    }
    
    /* Test complex CFG */
    for (int i = 0; i < 8; i++) {
        total += test_complex_cfg(i);
    }
    
#ifdef __SSE__
    /* Test vector pressure if available */
    __m128 vec_seed = _mm_set1_ps(1.0f);
    __m128 vec_result = test_vector_pressure(vec_seed);
    float vec_sum[4];
    _mm_store_ps(vec_sum, vec_result);
    total += (int)(vec_sum[0] + vec_sum[1] + vec_sum[2] + vec_sum[3]);
#endif
    
    printf("Total result: %d\n", total);
    return total != 0 ? 0 : 1;
}
