/* test_caller_save.c */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline))

/* External function declarations */
NOINLINE void foo(void);
NOINLINE void bar(int, double);
NOINLINE void baz(__m128, __m128);

/* Helper function to use many integer registers */
NOINLINE int pressure_integer(int a, int b, int c, int d) {
    /* Create many live integer variables across a call */
    volatile int v0 = a + 1;
    register int r1 = v0 * 2 + b;
    register int r2 = r1 + c * 3;
    register int r3 = r2 - d / 4;
    register int r4 = r3 ^ a;
    register int r5 = r4 | b;
    register int r6 = r5 & c;
    register int r7 = r6 << 2;
    register int r8 = r7 >> 1;
    register int r9 = r8 + d;
    register int r10 = r9 - a;
    register int r11 = r10 * b;
    register int r12 = r11 / (c + 1);
    register int r13 = r12 % (d + 1);
    register int r14 = r13 ^ r1;
    register int r15 = r14 | r2;
    register int r16 = r15 & r3;
    register int r17 = r16 << 3;
    register int r18 = r17 >> 2;
    register int r19 = r18 + r4;
    register int r20 = r19 - r5;
    
    /* Clobber many caller-saved registers with inline asm */
    asm volatile("" : : : 
        "rax", "rcx", "rdx", "rsi", "rdi", 
        "r8", "r9", "r10", "r11", "r12", 
        "r13", "r14", "r15", "xmm0", "xmm1",
        "xmm2", "xmm3", "xmm4", "xmm5", "xmm6",
        "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
        "xmm12", "xmm13", "xmm14", "xmm15");
    
    /* Call at what might be end of basic block */
    foo();
    
    /* Use all variables after call */
    return v0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 +
           r10 + r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18 + r19 + r20;
}

/* Function to pressure floating-point registers */
NOINLINE double pressure_float(double x, double y) {
    /* Many FP variables that must survive across call */
    volatile double v0 = sin(x);
    double d1 = cos(y);
    double d2 = v0 * d1;
    double d3 = d2 + x;
    double d4 = d3 - y;
    double d5 = d4 * v0;
    double d6 = d5 / d1;
    double d7 = sin(d6);
    double d8 = cos(d7);
    double d9 = d8 + d2;
    double d10 = d9 * d3;
    double d11 = d10 - d4;
    double d12 = d11 / d5;
    double d13 = sin(d12);
    double d14 = cos(d13);
    double d15 = d14 + d6;
    double d16 = d15 * d7;
    double d17 = d16 - d8;
    double d18 = d17 / d9;
    double d19 = sin(d18);
    double d20 = cos(d19);
    
    /* Complex control flow to create basic block ending with call */
    if (x > y) {
        /* This call is at the end of this basic block */
        bar((int)x, y);
    } else {
        /* Different path */
        bar((int)y, x);
    }
    
    /* Use FP variables after call */
    return v0 + d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 +
           d10 + d11 + d12 + d13 + d14 + d15 + d16 + d17 + d18 + d19 + d20;
}

/* Function to pressure vector registers */
NOINLINE __m128 pressure_vector(__m128 a, __m128 b) {
    /* Many vector variables */
    __m128 v0 = _mm_add_ps(a, b);
    __m128 v1 = _mm_mul_ps(v0, a);
    __m128 v2 = _mm_sub_ps(v1, b);
    __m128 v3 = _mm_add_ps(v2, v0);
    __m128 v4 = _mm_mul_ps(v3, v1);
    __m128 v5 = _mm_sub_ps(v4, v2);
    __m128 v6 = _mm_add_ps(v5, v3);
    __m128 v7 = _mm_mul_ps(v6, v4);
    __m128 v8 = _mm_sub_ps(v7, v5);
    __m128 v9 = _mm_add_ps(v8, v6);
    __m128 v10 = _mm_mul_ps(v9, v7);
    
    /* Switch statement creates multiple basic blocks */
    int choice = (int)(_mm_cvtss_f32(a) * 100) % 4;
    switch (choice) {
        case 0:
            /* Call at end of this case's basic block */
            baz(v0, v1);
            break;
        case 1:
            baz(v2, v3);
            break;
        case 2:
            baz(v4, v5);
            break;
        case 3:
            baz(v6, v7);
            break;
    }
    
    /* Use vectors after call */
    return _mm_add_ps(_mm_add_ps(v0, v1), 
                     _mm_add_ps(_mm_add_ps(v2, v3),
                               _mm_add_ps(_mm_add_ps(v4, v5),
                                         _mm_add_ps(_mm_add_ps(v6, v7),
                                                   _mm_add_ps(v8, v9))))));
}

/* Function with loop creating block ending with call */
NOINLINE int pressure_loop(int n) {
    int sum = 0;
    volatile int v[20];
    
    /* Partially unrolled loop with call at end of iteration */
    for (int i = 0; i < n; i++) {
        /* Many live variables in loop */
        register int r0 = i * 2;
        register int r1 = r0 + 1;
        register int r2 = r1 * 3;
        register int r3 = r2 - i;
        register int r4 = r3 ^ r0;
        register int r5 = r4 | r1;
        register int r6 = r5 & r2;
        register int r7 = r6 << 1;
        register int r8 = r7 >> 1;
        
        /* Call at what might be end of loop body basic block */
        if (i % 3 == 0) {
            foo();
        }
        
        /* Use variables after call */
        v[i % 20] = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8;
        sum += v[i % 20];
    }
    
    return sum;
}

/* Main function that calls all pressure functions */
int main(void) {
    int total = 0;
    
    /* Test integer pressure */
    total += pressure_integer(1, 2, 3, 4);
    
    /* Test FP pressure */
    total += (int)pressure_float(1.0, 2.0);
    
    /* Test vector pressure */
    __m128 a = _mm_set_ps(1.0, 2.0, 3.0, 4.0);
    __m128 b = _mm_set_ps(5.0, 6.0, 7.0, 8.0);
    __m128 c = pressure_vector(a, b);
    float f[4];
    _mm_store_ps(f, c);
    total += (int)(f[0] + f[1] + f[2] + f[3]);
    
    /* Test loop pressure */
    total += pressure_loop(10);
    
    printf("Result: %d\n", total);
    return 0;
}
