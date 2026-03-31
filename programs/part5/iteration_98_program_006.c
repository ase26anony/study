/* test_caller_save.c */
#include <stdio.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline))

/* External function that clobbers registers */
NOINLINE void foo(void);
NOINLINE void bar(int, double);
NOINLINE void baz(__m128, __m128);

/* Helper functions in separate compilation unit */
extern void helper1(void);
extern void helper2(int);

/* Global volatile to prevent optimization */
volatile int global_counter = 0;

/* Function to create integer register pressure */
NOINLINE int test_integer_pressure(int x, int y) {
    /* Create many integer live variables across a call */
    volatile int v0 = x + 1;
    register int r1 = v0 * 2;
    register int r2 = r1 + y;
    register int r3 = r2 ^ x;
    register int r4 = r3 * 3;
    register int r5 = r4 - y;
    register int r6 = r5 & 0xFF;
    register int r7 = r6 | 0x100;
    register int r8 = r7 << 2;
    register int r9 = r8 >> 1;
    register int r10 = r9 + x;
    register int r11 = r10 * y;
    register int r12 = r11 % 17;
    register int r13 = r12 ^ r1;
    register int r14 = r13 + r2;
    register int r15 = r14 * r3;
    register int r16 = r15 - r4;
    register int r17 = r16 & r5;
    register int r18 = r17 | r6;
    register int r19 = r18 << 3;
    register int r20 = r19 >> 2;
    
    /* Inline assembly to clobber caller-saved integer registers */
    asm volatile("" : : : 
        "rax", "rcx", "rdx", "rsi", "rdi", 
        "r8", "r9", "r10", "r11", "r12",
        "r13", "r14", "r15", "mm0", "mm1");
    
    /* Call at potential block end - inside if/else */
    if (x > y) {
        /* More computations to increase pressure */
        register int t1 = r20 + 1;
        register int t2 = t1 * 2;
        register int t3 = t2 + r1;
        register int t4 = t3 ^ r2;
        
        /* Call that might be at block end */
        foo();
        
        /* Use all variables after call */
        return v0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 +
               r10 + r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18 + 
               r19 + r20 + t1 + t2 + t3 + t4;
    } else {
        /* Alternative path */
        bar(x, y * 1.5);
        return r20 + 1;
    }
}

/* Function to create FP register pressure */
NOINLINE double test_fp_pressure(double a, double b) {
    /* Many FP live variables */
    volatile double v0 = a + 1.0;
    double d1 = sin(v0);
    double d2 = cos(d1);
    double d3 = d2 * b;
    double d4 = d3 + a;
    double d5 = sin(d4);
    double d6 = cos(d5);
    double d7 = d6 * 2.0;
    double d8 = d7 - b;
    double d9 = sin(d8);
    double d10 = cos(d9);
    double d11 = d10 * 3.0;
    double d12 = d11 + a;
    double d13 = sin(d12);
    double d14 = cos(d13);
    double d15 = d14 * 4.0;
    double d16 = d15 - b;
    double d17 = sin(d16);
    double d18 = cos(d17);
    double d19 = d18 * 5.0;
    double d20 = d19 + a;
    
    /* Clobber FP registers */
    asm volatile("" : : : 
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4",
        "xmm5", "xmm6", "xmm7", "xmm8", "xmm9",
        "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15");
    
    /* Call in switch case - creates complex CFG */
    switch ((int)a % 3) {
        case 0:
            /* Call at end of this case block */
            foo();
            /* Use variables after call */
            return d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10 +
                   d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19 + d20;
        case 1:
            bar((int)a, b);
            return d1 * d2;
        default:
            helper1();
            return d20;
    }
}

/* Function to create vector register pressure */
NOINLINE __m128 test_vector_pressure(__m128 v1, __m128 v2) {
    /* Many vector variables */
    __m128 vec0 = _mm_add_ps(v1, v2);
    __m128 vec1 = _mm_mul_ps(vec0, v1);
    __m128 vec2 = _mm_sub_ps(vec1, v2);
    __m128 vec3 = _mm_add_ps(vec2, vec0);
    __m128 vec4 = _mm_mul_ps(vec3, vec1);
    __m128 vec5 = _mm_sub_ps(vec4, vec2);
    __m128 vec6 = _mm_add_ps(vec5, vec3);
    __m128 vec7 = _mm_mul_ps(vec6, vec4);
    __m128 vec8 = _mm_sub_ps(vec7, vec5);
    __m128 vec9 = _mm_add_ps(vec8, vec6);
    __m128 vec10 = _mm_mul_ps(vec9, vec7);
    __m128 vec11 = _mm_sub_ps(vec10, vec8);
    __m128 vec12 = _mm_add_ps(vec11, vec9);
    
    /* Clobber vector registers */
    asm volatile("" : : : 
        "ymm0", "ymm1", "ymm2", "ymm3", "ymm4",
        "ymm5", "ymm6", "ymm7", "ymm8", "ymm9",
        "ymm10", "ymm11", "ymm12", "ymm13", "ymm14", "ymm15");
    
    /* Call in loop with partial unrolling */
    for (int i = 0; i < 4; i++) {
        if (i == 2) {
            /* Call at end of unrolled iteration block */
            baz(vec0, vec1);
            /* Use variables after call */
            __m128 t1 = _mm_add_ps(vec2, vec3);
            __m128 t2 = _mm_add_ps(t1, vec4);
            vec0 = _mm_add_ps(vec0, t2);
        } else {
            helper2(i);
        }
    }
    
    return _mm_add_ps(vec0, _mm_add_ps(vec1, _mm_add_ps(vec2, vec3)));
}

/* Complex function with mixed pressure */
NOINLINE int test_mixed_pressure(int x, double y, __m128 z) {
    /* Mixed register pressure */
    volatile int vi = x;
    volatile double vd = y;
    __m128 vv = z;
    
    int ri1 = vi * 2;
    int ri2 = ri1 + 1;
    int ri3 = ri2 ^ vi;
    int ri4 = ri3 * 3;
    
    double rd1 = sin(vd);
    double rd2 = cos(rd1);
    double rd3 = rd2 * 2.0;
    double rd4 = rd3 + vd;
    
    __m128 rv1 = _mm_add_ps(vv, _mm_set1_ps(1.0f));
    __m128 rv2 = _mm_mul_ps(rv1, vv);
    __m128 rv3 = _mm_add_ps(rv2, rv1);
    
    /* Call with many live variables across it */
    if (vi > 0) {
        /* More computations */
        int ri5 = ri4 + vi;
        double rd5 = rd4 * rd1;
        __m128 rv4 = _mm_add_ps(rv3, rv2);
        
        /* This call should trigger save/restore insertion */
        foo();
        
        /* Use all variables after call */
        return ri1 + ri2 + ri3 + ri4 + ri5 + 
               (int)(rd1 + rd2 + rd3 + rd4 + rd5) +
               (int)(((float*)&rv1)[0] + ((float*)&rv2)[0] + 
                     ((float*)&rv3)[0] + ((float*)&rv4)[0]);
    } else {
        bar(vi, vd);
        return 0;
    }
}

/* Main function that calls all test cases */
int main(void) {
    int result = 0;
    
    /* Test integer pressure */
    result += test_integer_pressure(global_counter++, 42);
    
    /* Test FP pressure */
    result += (int)test_fp_pressure(3.14159, 2.71828);
    
    /* Test vector pressure */
    __m128 v1 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 v2 = _mm_set_ps(5.0f, 6.0f, 7.0f, 8.0f);
    __m128 vres = test_vector_pressure(v1, v2);
    result += (int)(((float*)&vres)[0]);
    
    /* Test mixed pressure */
    result += test_mixed_pressure(global_counter, 1.2345, v1);
    
    /* Ensure result is used */
    printf("Result: %d\n", result);
    
    return 0;
}
