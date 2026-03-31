/* test_caller_save.c - Forces GCC to insert save/restore instructions at call sites */

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline))

/* Helper function in separate compilation unit */
NOINLINE void foo(void);
NOINLINE void bar(int, double);
NOINLINE void baz(void*, void*);

/* Complex function with integer register pressure */
NOINLINE int test_integer_pressure(int seed) {
    volatile int v0 = seed;  /* Prevent optimization */
    register int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9;
    register int r10, r11, r12, r13, r14, r15, r16, r17, r18, r19;
    
    /* Create long dependency chain to force register usage */
    r0 = v0 + 1;
    r1 = r0 * 2 - seed;
    r2 = r1 + r0 * 3;
    r3 = r2 ^ r1;
    r4 = r3 * 7 + r2;
    r5 = r4 / 2 + r3;
    r6 = r5 << 3;
    r7 = r6 | r5;
    r8 = r7 - r6;
    r9 = r8 * r7;
    r10 = r9 + r8;
    r11 = r10 ^ r9;
    r12 = r11 * 11;
    r13 = r12 + r11;
    r14 = r13 & r12;
    r15 = r14 | r13;
    r16 = r15 - r14;
    r17 = r16 * r15;
    r18 = r17 + 42;
    r19 = r18 ^ r17;
    
    /* Clobber many caller-saved registers */
    asm volatile("" : : : 
        "rax", "rcx", "rdx", "rsi", "rdi", 
        "r8", "r9", "r10", "r11", 
        "xmm0", "xmm1", "xmm2", "xmm3",
        "xmm4", "xmm5", "xmm6", "xmm7",
        "xmm8", "xmm9", "xmm10", "xmm11",
        "xmm12", "xmm13", "xmm14", "xmm15");
    
    /* Call at potential block end */
    if (seed % 2) {
        /* This creates a basic block ending with foo() */
        foo();
    } else {
        /* Alternative path */
        bar(r19, (double)r18);
    }
    
    /* Use all variables after call to keep them live */
    return r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 +
           r10 + r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18 + r19;
}

/* Function with floating-point pressure */
NOINLINE double test_fp_pressure(double seed) {
    volatile double vd = seed;
    double d0, d1, d2, d3, d4, d5, d6, d7, d8, d9;
    double d10, d11, d12, d13, d14, d15, d16, d17, d18, d19;
    
    /* Complex FP computations */
    d0 = sin(vd);
    d1 = cos(d0);
    d2 = d0 * d1 + vd;
    d3 = sin(d2) * cos(d1);
    d4 = d3 * d2 - d1;
    d5 = exp(d4);
    d6 = log(fabs(d5) + 1.0);
    d7 = d6 * d5 / d4;
    d8 = sin(d7) + cos(d6);
    d9 = d8 * d7 - d6;
    d10 = tan(d9);
    d11 = atan(d10);
    d12 = d11 * d10 + d9;
    d13 = sin(d12) * cos(d11);
    d14 = d13 * d12 - d11;
    d15 = exp(d14);
    d16 = log(fabs(d15) + 1.0);
    d17 = d16 * d15 / d14;
    d18 = sin(d17) + cos(d16);
    d19 = d18 * d17 - d16;
    
    /* Switch statement to create multiple basic blocks */
    int choice = (int)seed % 4;
    switch (choice) {
        case 0:
            /* Call at end of this case's basic block */
            foo();
            break;
        case 1:
            bar((int)d19, d18);
            break;
        case 2:
            /* Another call at block end */
            baz(&d17, &d16);
            break;
        default:
            /* Complex computation then call */
            d19 = sin(d19) * 2.0;
            foo();
    }
    
    /* Keep all FP values live */
    return d0 + d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 +
           d10 + d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19;
}

#ifdef __SSE2__
/* Function with vector register pressure */
NOINLINE __m128 test_vector_pressure(__m128 seed) {
    typedef float v4sf __attribute__((vector_size(16)));
    v4sf v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    
    /* Initialize with broadcast */
    float f = ((float*)&seed)[0];
    v0 = (v4sf){f, f+1, f+2, f+3};
    v1 = v0 * (v4sf){2, 3, 4, 5};
    v2 = v1 + v0;
    v3 = v2 * v1;
    v4 = v3 - v2;
    v5 = v4 * (v4sf){1.5, 2.5, 3.5, 4.5};
    v6 = v5 + v4;
    v7 = v6 * v5;
    v8 = v7 - v6;
    v9 = v8 * (v4sf){0.5, 1.5, 2.5, 3.5};
    
    /* Loop with call at end of unrolled iteration */
    for (int i = 0; i < 3; i++) {
        if (i == 1) {
            /* Call at end of this basic block */
            foo();
        } else {
            /* More vector operations */
            v0 = v0 + v1;
            v2 = v2 * v3;
        }
        
        /* Additional vector computations */
        v4 = v4 + v5;
        v6 = v6 * v7;
    }
    
    /* Use all vectors */
    __m128 result = (__m128)(v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9);
    return result;
}
#endif

/* Mixed pressure in complex control flow */
NOINLINE int test_mixed_pressure(int mode) {
    int result = 0;
    
    /* Create multiple basic blocks with calls at different points */
    for (int i = 0; i < 10; i++) {
        if (i % 3 == 0) {
            /* Integer pressure in this path */
            int a = i * 2, b = i + 1, c = a ^ b;
            int d = c * 3, e = d >> 1, f = e | d;
            
            /* Call with many live integer values */
            if (i == 6) {
                /* This call should be at block end */
                foo();
            }
            
            result += a + b + c + d + e + f;
        } else if (i % 3 == 1) {
            /* FP pressure in this path */
            double x = sin(i), y = cos(i);
            double z = x * y + i;
            
            /* Call with live FP values */
            bar(i, z);
            
            result += (int)(x + y + z);
        } else {
            /* Memory pressure */
            char buffer[256];
            void* ptr1 = &buffer[0];
            void* ptr2 = &buffer[128];
            
            /* Call with pointer arguments */
            baz(ptr1, ptr2);
            
            result += buffer[0];
        }
    }
    
    return result;
}

/* Main driver */
int main(void) {
    int total = 0;
    
    /* Test different pressure scenarios */
    total += test_integer_pressure(42);
    
    double fp_result = test_fp_pressure(3.14159);
    total += (int)fp_result;
    
    #ifdef __SSE2__
    __m128 vec_seed = _mm_set_ps(1.0, 2.0, 3.0, 4.0);
    __m128 vec_result = test_vector_pressure(vec_seed);
    total += ((int*)&vec_result)[0];
    #endif
    
    total += test_mixed_pressure(2);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return 0;
}
