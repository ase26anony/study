/* test_caller_save.c */
#include <stdio.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Prevent inlining of called functions */
#define NOINLINE __attribute__((noinline))

/* External function declarations */
NOINLINE void foo(void);
NOINLINE void bar(int, double);
NOINLINE void baz(__m128);

/* Helper function to use many integer registers */
NOINLINE int pressure_integer(int seed) {
    volatile int a = seed;
    volatile int b = seed + 1;
    volatile int c = seed + 2;
    volatile int d = seed + 3;
    volatile int e = seed + 4;
    volatile int f = seed + 5;
    volatile int g = seed + 6;
    volatile int h = seed + 7;
    volatile int i = seed + 8;
    volatile int j = seed + 9;
    volatile int k = seed + 10;
    volatile int l = seed + 11;
    volatile int m = seed + 12;
    volatile int n = seed + 13;
    volatile int o = seed + 14;
    volatile int p = seed + 15;
    volatile int q = seed + 16;
    volatile int r = seed + 17;
    volatile int s = seed + 18;
    volatile int t = seed + 19;
    
    /* Create complex dependency chain */
    int r0 = a + b;
    int r1 = r0 * c;
    int r2 = r1 - d;
    int r3 = r2 + e;
    int r4 = r3 * f;
    int r5 = r4 - g;
    int r6 = r5 + h;
    int r7 = r6 * i;
    int r8 = r7 - j;
    int r9 = r8 + k;
    int r10 = r9 * l;
    int r11 = r10 - m;
    int r12 = r11 + n;
    int r13 = r12 * o;
    int r14 = r13 - p;
    int r15 = r14 + q;
    int r16 = r15 * r;
    int r17 = r16 - s;
    int r18 = r17 + t;
    
    /* Call with all these live variables */
    foo();
    
    /* Use results after call */
    return r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 +
           r10 + r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18;
}

/* Helper function to use many floating-point registers */
NOINLINE double pressure_float(double seed) {
    volatile double a = seed;
    volatile double b = seed * 1.1;
    volatile double c = seed * 1.2;
    volatile double d = seed * 1.3;
    volatile double e = seed * 1.4;
    volatile double f = seed * 1.5;
    volatile double g = seed * 1.6;
    volatile double h = seed * 1.7;
    volatile double i = seed * 1.8;
    volatile double j = seed * 1.9;
    volatile double k = seed * 2.0;
    volatile double l = seed * 2.1;
    volatile double m = seed * 2.2;
    volatile double n = seed * 2.3;
    volatile double o = seed * 2.4;
    
    /* Complex FP computations */
    double f0 = sin(a) + cos(b);
    double f1 = f0 * tan(c);
    double f2 = f1 + exp(d);
    double f3 = f2 * log(e + 1.0);
    double f4 = f3 - sin(f);
    double f5 = f4 * cos(g);
    double f6 = f5 + tan(h);
    double f7 = f6 * exp(i);
    double f8 = f7 - log(j + 1.0);
    double f9 = f8 + sin(k);
    double f10 = f9 * cos(l);
    double f11 = f10 + tan(m);
    double f12 = f11 * exp(n);
    double f13 = f12 - log(o + 1.0);
    
    /* Call with FP registers live */
    bar(42, f13);
    
    /* Use results after call */
    return f0 + f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 +
           f10 + f11 + f12 + f13;
}

/* Helper function to use vector registers */
NOINLINE __m128 pressure_vector(float seed) {
    volatile __m128 v0 = _mm_set_ps(seed, seed+1.0f, seed+2.0f, seed+3.0f);
    volatile __m128 v1 = _mm_set_ps(seed+4.0f, seed+5.0f, seed+6.0f, seed+7.0f);
    volatile __m128 v2 = _mm_set_ps(seed+8.0f, seed+9.0f, seed+10.0f, seed+11.0f);
    volatile __m128 v3 = _mm_set_ps(seed+12.0f, seed+13.0f, seed+14.0f, seed+15.0f);
    volatile __m128 v4 = _mm_set_ps(seed+16.0f, seed+17.0f, seed+18.0f, seed+19.0f);
    volatile __m128 v5 = _mm_set_ps(seed+20.0f, seed+21.0f, seed+22.0f, seed+23.0f);
    volatile __m128 v6 = _mm_set_ps(seed+24.0f, seed+25.0f, seed+26.0f, seed+27.0f);
    volatile __m128 v7 = _mm_set_ps(seed+28.0f, seed+29.0f, seed+30.0f, seed+31.0f);
    
    /* Vector operations */
    __m128 r0 = _mm_add_ps(v0, v1);
    __m128 r1 = _mm_mul_ps(r0, v2);
    __m128 r2 = _mm_sub_ps(r1, v3);
    __m128 r3 = _mm_add_ps(r2, v4);
    __m128 r4 = _mm_mul_ps(r3, v5);
    __m128 r5 = _mm_sub_ps(r4, v6);
    __m128 r6 = _mm_add_ps(r5, v7);
    
    /* Call with vector registers live */
    baz(r6);
    
    /* Use results after call */
    return _mm_add_ps(_mm_add_ps(r0, r1), 
                     _mm_add_ps(_mm_add_ps(r2, r3), 
                               _mm_add_ps(_mm_add_ps(r4, r5), r6)));
}

/* Function with call at end of basic block in if-else */
NOINLINE int call_at_block_end(int x, int y) {
    int result = 0;
    
    /* Complex condition creating multiple basic blocks */
    if (x > 0) {
        /* This block will end with the call to foo() */
        volatile int a = x * 2;
        volatile int b = y * 3;
        volatile int c = a + b;
        volatile int d = c * x;
        volatile int e = d - y;
        volatile int f = e / 2;
        volatile int g = f + x;
        volatile int h = g * y;
        volatile int i = h - a;
        volatile int j = i + b;
        
        /* Clobber many registers before call */
        asm volatile("" : : : 
            "rax", "rcx", "rdx", "rsi", "rdi", 
            "r8", "r9", "r10", "r11",
            "xmm0", "xmm1", "xmm2", "xmm3",
            "xmm4", "xmm5", "xmm6", "xmm7",
            "xmm8", "xmm9", "xmm10", "xmm11",
            "xmm12", "xmm13", "xmm14", "xmm15");
        
        /* Call at what might be block end */
        foo();
        
        result = j;
    } else {
        /* Alternative path */
        result = x + y;
    }
    
    return result;
}

/* Function with call in switch case */
NOINLINE int call_in_switch(int x) {
    int result = 0;
    
    switch (x % 4) {
        case 0: {
            /* High register pressure in this case */
            volatile double d0 = sin(x);
            volatile double d1 = cos(x);
            volatile double d2 = d0 * d1;
            volatile double d3 = d2 + x;
            volatile double d4 = d3 * 2.0;
            volatile double d5 = d4 - 1.0;
            
            /* Call that might be at block end */
            bar(x, d5);
            result = (int)d5;
            break;
        }
        case 1:
            result = x * 2;
            break;
        case 2:
            result = x * 3;
            break;
        case 3:
            result = x * 4;
            break;
    }
    
    return result;
}

/* Function with call in unrolled loop */
NOINLINE int call_in_unrolled_loop(int n) {
    int sum = 0;
    
    /* Partially unrolled loop */
    for (int i = 0; i < n; i += 4) {
        volatile int a = i;
        volatile int b = i + 1;
        volatile int c = i + 2;
        volatile int d = i + 3;
        
        volatile int r0 = a * b;
        volatile int r1 = r0 + c;
        volatile int r2 = r1 - d;
        volatile int r3 = r2 * a;
        volatile int r4 = r3 + b;
        volatile int r5 = r4 - c;
        volatile int r6 = r5 * d;
        volatile int r7 = r6 + a;
        
        /* Call inside loop - might be at block end */
        foo();
        
        sum += r7;
    }
    
    return sum;
}

int main(void) {
    int total = 0;
    
    /* Test different pressure scenarios */
    total += pressure_integer(42);
    total += (int)pressure_float(3.14);
    
    __m128 vec_result = pressure_vector(1.0f);
    float vec_sum[4];
    _mm_store_ps(vec_sum, vec_result);
    total += (int)(vec_sum[0] + vec_sum[1] + vec_sum[2] + vec_sum[3]);
    
    total += call_at_block_end(10, 20);
    total += call_in_switch(7);
    total += call_in_unrolled_loop(8);
    
    printf("Result: %d\n", total);
    return 0;
}
