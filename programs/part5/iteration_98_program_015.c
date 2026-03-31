/* test_caller_save.c - Forces GCC to insert save/restore instructions at call sites */

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline, noipa))

/* Helper function in separate compilation unit */
NOINLINE void foo(void);
NOINLINE void bar(int, double);
NOINLINE void baz(void*, void*);

/* Vector types for SSE/AVX pressure */
typedef float v4sf __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef double v2df __attribute__((vector_size(16)));
typedef double v4df __attribute__((vector_size(32)));

/* Global volatile variables to prevent optimization */
volatile int global_counter = 0;
volatile double global_sum = 0.0;

/* ========== Test Case 1: Integer register pressure at block end ========== */
NOINLINE int test_integer_pressure(int seed) {
    /* Create massive integer register pressure */
    register int r0  = seed + 1;
    register int r1  = r0 * 2;
    register int r2  = r1 + seed;
    register int r3  = r2 ^ r1;
    register int r4  = r3 - r0;
    register int r5  = r4 | r2;
    register int r6  = r5 & r3;
    register int r7  = r6 << 2;
    register int r8  = r7 >> 1;
    register int r9  = r8 + r4;
    register int r10 = r9 * 3;
    register int r11 = r10 - r5;
    register int r12 = r11 ^ r6;
    register int r13 = r12 | r7;
    register int r14 = r13 & r8;
    register int r15 = r14 + r9;
    register int r16 = r15 * 5;
    register int r17 = r16 - r10;
    register int r18 = r17 ^ r11;
    register int r19 = r18 | r12;
    register int r20 = r19 & r13;
    
    /* Volatile variables that must survive across call */
    volatile int v0 = r0;
    volatile int v1 = r1;
    volatile int v2 = r2;
    volatile int v3 = r3;
    volatile int v4 = r4;
    volatile int v5 = r5;
    
    /* Clobber many caller-saved registers */
    asm volatile("" : : : 
        "rax", "rcx", "rdx", "rsi", "rdi", 
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
        "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
        "xmm12", "xmm13", "xmm14", "xmm15");
    
    /* Call at potential block end - inside if/else */
    int result;
    if (seed % 3 == 0) {
        /* This creates a basic block ending with the call */
        foo();  /* Non-inlineable call */
        
        /* Use all the volatile variables after call */
        result = v0 + v1 + v2 + v3 + v4 + v5 + 
                 r14 + r15 + r16 + r17 + r18 + r19 + r20;
    } else if (seed % 3 == 1) {
        result = seed * 2;
    } else {
        result = seed / 2;
    }
    
    return result + r0 - r20;
}

/* ========== Test Case 2: FP register pressure with math functions ========== */
NOINLINE double test_fp_pressure(double angle) {
    /* Create FP register pressure with math functions */
    double a0 = sin(angle);
    double a1 = cos(angle);
    double a2 = tan(angle);
    double a3 = sin(angle * 2.0);
    double a4 = cos(angle * 2.0);
    double a5 = tan(angle * 2.0);
    double a6 = sin(angle * 3.0);
    double a7 = cos(angle * 3.0);
    double a8 = tan(angle * 3.0);
    double a9 = sin(angle * 4.0);
    double a10 = cos(angle * 4.0);
    double a11 = tan(angle * 4.0);
    double a12 = sin(angle * 5.0);
    double a13 = cos(angle * 5.0);
    double a14 = tan(angle * 5.0);
    double a15 = sin(angle * 6.0);
    double a16 = cos(angle * 6.0);
    double a17 = tan(angle * 6.0);
    double a18 = sin(angle * 7.0);
    double a19 = cos(angle * 7.0);
    
    /* Volatile FP variables */
    volatile double vd0 = a0;
    volatile double vd1 = a1;
    volatile double vd2 = a2;
    volatile double vd3 = a3;
    volatile double vd4 = a4;
    
    /* Switch creates multiple basic blocks */
    int choice = (int)angle % 4;
    double result = 0.0;
    
    switch (choice) {
        case 0:
            /* Call at end of this case's block */
            bar((int)angle, a0);
            result = vd0 + vd1 + vd2 + vd3 + vd4 +
                    a5 + a6 + a7 + a8 + a9 + a10 + a11 + a12 + 
                    a13 + a14 + a15 + a16 + a17 + a18 + a19;
            break;
        case 1:
            result = a0 + a1;
            break;
        case 2:
            result = a2 + a3;
            break;
        case 3:
            result = a4 + a5;
            break;
    }
    
    return result;
}

/* ========== Test Case 3: Vector register pressure ========== */
NOINLINE float test_vector_pressure(float x) {
    /* Create many vector variables */
    v4sf v0 = {x, x+1, x+2, x+3};
    v4sf v1 = v0 * (v4sf){2.0f, 2.0f, 2.0f, 2.0f};
    v4sf v2 = v1 + (v4sf){1.0f, 2.0f, 3.0f, 4.0f};
    v4sf v3 = v2 * v0;
    v4sf v4 = v3 - v1;
    v4sf v5 = v4 + v2;
    v4sf v6 = v5 * (v4sf){0.5f, 0.5f, 0.5f, 0.5f};
    v4sf v7 = v6 + v3;
    v4sf v8 = v7 - v4;
    v4sf v9 = v8 * v5;
    v4sf v10 = v9 + v6;
    v4sf v11 = v10 - v7;
    v4sf v12 = v11 * v8;
    v4sf v13 = v12 + v9;
    v4sf v14 = v13 - v10;
    v4sf v15 = v14 * v11;
    
    /* AVX vectors if available */
    v8sf w0 = {x, x+1, x+2, x+3, x+4, x+5, x+6, x+7};
    v8sf w1 = w0 * (v8sf){2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f};
    v8sf w2 = w1 + (v8sf){1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    
    /* Volatile vector */
    volatile v4sf vv0 = v0;
    
    /* Loop with call at end of unrolled iteration */
    float sum = 0.0f;
    for (int i = 0; i < 3; i++) {
        if (i == 1) {
            /* Call inside loop, at end of basic block */
            float temp[4];
            memcpy(temp, &vv0, sizeof(v4sf));
            baz(temp, &sum);
            
            /* Use vectors after call */
            v0 = v0 + v1;
            v2 = v2 * v3;
        }
        sum += v0[i % 4] + v1[i % 4] + v2[i % 4] + 
               v3[i % 4] + v4[i % 4] + v5[i % 4];
    }
    
    return sum;
}

/* ========== Test Case 4: Mixed pressure in complex CFG ========== */
NOINLINE double test_mixed_pressure(int mode, double x) {
    double result = 0.0;
    
    /* Complex control flow with nested conditionals */
    if (mode > 0) {
        /* Integer pressure */
        int i0 = mode * 2;
        int i1 = i0 + 1;
        int i2 = i1 * 3;
        int i3 = i2 - i0;
        int i4 = i3 ^ i1;
        int i5 = i4 | i2;
        volatile int vi0 = i0;
        volatile int vi1 = i1;
        
        if (mode % 2 == 0) {
            /* FP pressure */
            double d0 = sin(x);
            double d1 = cos(x);
            double d2 = exp(x);
            double d3 = log(fabs(x) + 1.0);
            volatile double vd0 = d0;
            volatile double vd1 = d1;
            
            /* Call at end of this inner block */
            bar(mode, x);
            
            result = vi0 + vi1 + vd0 + vd1 + d2 + d3;
        } else {
            result = x * 2.0;
        }
        
        /* More computations using the volatiles */
        result += vi0 - vi1;
    } else {
        result = x / 2.0;
    }
    
    return result;
}

/* ========== Main driver ========== */
int main(void) {
    int total = 0;
    double sum = 0.0;
    
    /* Test all scenarios to increase coverage chance */
    for (int i = 0; i < 10; i++) {
        total += test_integer_pressure(i);
        sum += test_fp_pressure(i * 0.1);
        sum += test_vector_pressure(i * 0.5f);
        sum += test_mixed_pressure(i, i * 0.2);
        
        /* Prevent loop optimization */
        asm volatile("" : "+r"(total), "+r"(sum));
    }
    
    /* Use results to prevent dead code elimination */
    global_counter = total;
    global_sum = sum;
    
    printf("Result: %d, %f\n", total, sum);
    return 0;
}
