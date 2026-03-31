/* test_caller_save.c - Forces GCC to insert save/restore instructions at call sites */

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline))

/* External function declarations - cannot be inlined */
NOINLINE void foo(void);
NOINLINE void bar(int, double);
NOINLINE double baz(double, double);
NOINLINE void clobber_many_regs(void);

/* Helper function in separate compilation unit */
extern void external_func(int, double, float);

/* Vector types for SSE/AVX pressure */
typedef float v4sf __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef double v2df __attribute__((vector_size(16)));
typedef double v4df __attribute__((vector_size(32)));

/* ============================================
   Test Case 1: Integer register pressure
   Creates many live integer variables across a call
   ============================================ */
NOINLINE int test_integer_pressure(int seed) {
    /* Declare many volatile integer variables to force register allocation */
    volatile int v0 = seed + 1;
    volatile int v1 = v0 * 2;
    volatile int v2 = v1 + seed;
    volatile int v3 = v2 * 3;
    volatile int v4 = v3 - seed;
    volatile int v5 = v4 / 2;
    volatile int v6 = v5 | 0xFF;
    volatile int v7 = v6 & 0x0F;
    volatile int v8 = v7 ^ 0x55;
    volatile int v9 = v8 << 2;
    volatile int v10 = v9 >> 1;
    volatile int v11 = v10 + 100;
    volatile int v12 = v11 * 2;
    volatile int v13 = v12 - 50;
    volatile int v14 = v13 | 0xAA;
    volatile int v15 = v14 & 0x55;
    volatile int v16 = v15 ^ 0xFF;
    volatile int v17 = v16 << 3;
    volatile int v18 = v17 >> 2;
    volatile int v19 = v18 + 999;
    volatile int v20 = v19 * 3;
    volatile int v21 = v20 / 4;
    volatile int v22 = v21 | 0xCC;
    volatile int v23 = v22 & 0x33;
    volatile int v24 = v23 ^ 0x99;
    volatile int v25 = v24 << 1;
    volatile int v26 = v25 >> 1;
    volatile int v27 = v26 + 777;
    volatile int v28 = v27 * 5;
    volatile int v29 = v28 - 333;
    volatile int v30 = v29 | 0xF0;
    
    /* Additional non-volatile variables for more pressure */
    register int r0 = v0 + v1;
    register int r1 = r0 * v2;
    register int r2 = r1 + v3;
    register int r3 = r2 * v4;
    register int r4 = r3 + v5;
    register int r5 = r4 * v6;
    register int r6 = r5 + v7;
    register int r7 = r6 * v8;
    register int r8 = r7 + v9;
    register int r9 = r8 * v10;
    
    /* Create complex control flow with call at block end */
    int result = 0;
    if (seed % 3 == 0) {
        /* This basic block ends with the call to foo() */
        
        /* Inline assembly to clobber caller-saved integer registers */
        asm volatile("" : : : 
            "rax", "rcx", "rdx", "rsi", "rdi", 
            "r8", "r9", "r10", "r11", "r12", 
            "r13", "r14", "r15", "xmm0", "xmm1",
            "xmm2", "xmm3", "xmm4", "xmm5", "xmm6",
            "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
            "xmm12", "xmm13", "xmm14", "xmm15");
        
        /* Call that forces save/restore - at end of basic block */
        foo();
        
        /* Use all variables after call - they must be preserved */
        result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                 v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30 +
                 r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9;
    } else if (seed % 3 == 1) {
        /* Alternative path - different basic block */
        result = seed * 2;
    } else {
        /* Another alternative path */
        result = seed / 2;
    }
    
    return result;
}

/* ============================================
   Test Case 2: Floating-point register pressure
   ============================================ */
NOINLINE double test_fp_pressure(double seed) {
    /* Many double variables to pressure FP registers */
    volatile double d0 = seed + 1.0;
    volatile double d1 = d0 * 2.0;
    volatile double d2 = sin(d1);
    volatile double d3 = cos(d2);
    volatile double d4 = d3 * 3.14159;
    volatile double d5 = exp(d4);
    volatile double d6 = log(fabs(d5) + 1.0);
    volatile double d7 = d6 * 2.71828;
    volatile double d8 = sqrt(fabs(d7));
    volatile double d9 = d8 * d0;
    volatile double d10 = d9 / d1;
    volatile double d11 = d10 + d2;
    volatile double d12 = d11 * d3;
    volatile double d13 = d12 - d4;
    volatile double d14 = d13 / d5;
    volatile double d15 = d14 + d6;
    volatile double d16 = d15 * d7;
    volatile double d17 = d16 - d8;
    volatile double d18 = d17 / d9;
    volatile double d19 = d18 + d10;
    volatile double d20 = d19 * d11;
    
    /* Additional FP computations */
    register double r0 = d0 + d1;
    register double r1 = r0 * d2;
    register double r2 = r1 + d3;
    register double r3 = r2 * d4;
    register double r4 = r3 + d5;
    register double r5 = r4 * d6;
    register double r6 = r5 + d7;
    register double r7 = r6 * d8;
    register double r8 = r7 + d9;
    register double r9 = r8 * d10;
    
    double result = 0.0;
    
    /* Switch statement creates multiple basic blocks */
    switch ((int)seed % 4) {
        case 0: {
            /* This case block ends with a call */
            
            /* Clobber FP/vector registers */
            asm volatile("" : : : 
                "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                "xmm12", "xmm13", "xmm14", "xmm15",
                "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5",
                "ymm6", "ymm7", "ymm8", "ymm9", "ymm10", "ymm11",
                "ymm12", "ymm13", "ymm14", "ymm15");
            
            /* Call at end of basic block */
            bar((int)seed, d0);
            
            /* Use all FP variables after call */
            result = d0 + d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 +
                     d10 + d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19 + d20 +
                     r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9;
            break;
        }
        case 1:
            result = d0 * d1;
            break;
        case 2:
            result = d2 / d3;
            break;
        default:
            result = d4 - d5;
            break;
    }
    
    return result;
}

/* ============================================
   Test Case 3: Vector register pressure (SSE/AVX)
   ============================================ */
NOINLINE float test_vector_pressure(float seed) {
    /* Many vector variables */
    v4sf v0 = {seed, seed+1.0f, seed+2.0f, seed+3.0f};
    v4sf v1 = v0 * (v4sf){2.0f, 3.0f, 4.0f, 5.0f};
    v4sf v2 = v1 + (v4sf){1.0f, 2.0f, 3.0f, 4.0f};
    v4sf v3 = v2 * (v4sf){0.5f, 1.5f, 2.5f, 3.5f};
    v4sf v4 = v3 - (v4sf){0.1f, 0.2f, 0.3f, 0.4f};
    v4sf v5 = v4 / (v4sf){2.0f, 3.0f, 4.0f, 5.0f};
    v4sf v6 = v5 + v0;
    v4sf v7 = v6 * v1;
    v4sf v8 = v7 - v2;
    v4sf v9 = v8 / v3;
    v4sf v10 = v9 + v4;
    
    /* AVX vectors if available */
    v8sf w0 = {seed, seed+1.0f, seed+2.0f, seed+3.0f,
               seed+4.0f, seed+5.0f, seed+6.0f, seed+7.0f};
    v8sf w1 = w0 * (v8sf){2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
    v8sf w2 = w1 + (v8sf){1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    
    /* Double precision vectors */
    v2df d0 = {seed, seed+1.0};
    v2df d1 = d0 * (v2df){2.0, 3.0};
    v2df d2 = d1 + (v2df){1.0, 2.0};
    
    float result = 0.0f;
    
    /* Loop with partial unrolling - call at end of unrolled block */
    for (int i = 0; i < 4; i++) {
        if (i == 2) {
            /* This creates a basic block ending with the call */
            
            /* Clobber vector registers */
            asm volatile("" : : : 
                "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                "xmm12", "xmm13", "xmm14", "xmm15",
                "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5",
                "ymm6", "ymm7", "ymm8", "ymm9", "ymm10", "ymm11",
                "ymm12", "ymm13", "ymm14", "ymm15");
            
            /* Call at end of basic block */
            double temp = baz((double)seed, (double)seed + 1.0);
            
            /* Use vector variables after call */
            v4sf tempv = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
            result += tempv[0] + tempv[1] + tempv[2] + tempv[3];
            result += (float)temp;
        } else {
            /* Other loop iterations */
            v0 = v0 * v1;
            v1 = v1 + v2;
        }
    }
    
    /* Use all vector variables */
    float sum = 0.0f;
    for (int i = 0; i < 4; i++) sum += v0[i] + v1[i] + v2[i] + v3[i] + v4[i];
    for (int i = 0; i < 8; i++) sum += w0[i] + w1[i] + w2[i];
    
    return result + sum;
}

/* ============================================
   Test Case 4: Mixed register pressure with complex CFG
   ============================================ */
NOINLINE double test_mixed_pressure(int mode, double x, float y) {
    volatile int vi0 = mode * 2;
    volatile int vi1 = vi0 + 1;
    volatile int vi2 = vi1 * 3;
    volatile int vi3 = vi2 - mode;
    volatile int vi4 = vi3 / 2;
    volatile int vi5 = vi4 | 0xFF;
    
    volatile double vd0 = x + 1.0;
    volatile double vd1 = sin(vd0);
    volatile double vd2 = cos(vd1);
    volatile double vd3 = vd2 * 3.14159;
    
    volatile float vf0 = y * 2.0f;
    volatile float vf1 = vf0 + 1.0f;
    volatile float vf2 = vf1 * 3.0f;
    volatile float vf3 = vf2 - y;
    
    v4sf vec0 = {vf0, vf1, vf2, vf3};
    v4sf vec1 = vec0 * (v4sf){2.0f, 3.0f, 4.0f, 5.0f};
    
    double result = 0.0;
    
    /* Nested control flow */
    if (mode > 0) {
        if (mode < 100) {
            for (int i = 0; i < 3; i++) {
                if (i == 1) {
                    /* Call at end of this basic block */
                    
                    /* Massive clobber list */
                    asm volatile("" : : : 
                        "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                        "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                        "xmm12", "xmm13", "xmm14", "xmm15",
                        "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5",
                        "ymm6", "ymm7", "ymm8", "ymm9", "ymm10", "ymm11",
                        "ymm12", "ymm13", "ymm14", "ymm15");
                    
                    /* External call - cannot be inlined */
                    external_func(vi0, vd0, vf0);
                    
                    /* Use all variables */
                    result = vi0 + vi1 + vi2 + vi3 + vi4 + vi5 +
                             vd0 + vd1 + vd2 + vd3 +
                             vf0 + vf1 + vf2 + vf3 +
                             vec0[0] + vec0[1] + vec0[2] + vec0[3] +
                             vec1[0] + vec1[1] + vec1[2] + vec1[3];
                } else {
                    vi0++;
                    vd0 *= 1.1;
                }
            }
        } else {
            result = x * y;
        }
    } else {
        result = x / (y + 1.0f);
    }
    
    return result;
}

/* ============================================
   Main function - calls all test cases
   ============================================ */
int main(void) {
    int total = 0;
    double sum = 0.0;
    
    /* Call all test functions with different parameters */
    total += test_integer_pressure(42);
    total += test_integer_pressure(123);
    total += test_integer_pressure(789);
    
    sum += test_fp_pressure(3.14159);
    sum += test_fp_pressure(2.71828);
    sum += test_fp_pressure(1.61803);
    
    sum += test_vector_pressure(1.0f);
    sum += test_vector_pressure(2.0f);
    sum += test_vector_pressure(3.0f);
    
    sum += test_mixed_pressure(1, 1.0, 2.0f);
    sum += test_mixed_pressure(50, 2.0, 3.0f);
    sum += test_mixed_pressure(99, 3.0, 4.0f);
    
    /* Prevent dead code elimination */
    volatile int print_total = total;
    volatile double print_sum = sum;
    
    printf("Result: %d, %f\n", print_total, print_sum);
    
    return 0;
}
