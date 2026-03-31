/* test_early_remat.c - Test program for GCC early rematerialization pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External non-inline functions to create register pressure points */
extern int ext_func1(int);
extern int ext_func2(int, int);
extern double ext_func3(double);
extern long long ext_func4(long long);

/* Dummy external functions to prevent inlining */
int __attribute__((noinline)) ext_func1(int x) { return x ^ 0x1234; }
int __attribute__((noinline)) ext_func2(int x, int y) { return x * y + 1; }
double __attribute__((noinline)) ext_func3(double x) { return x * 1.5; }
long long __attribute__((noinline)) ext_func4(long long x) { return x + 0xABCDEF; }

/* Vector type to test different modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High integer register pressure with complex expressions */
int __attribute__((noinline)) test_integer_pressure(int seed) {
    volatile int result = 0;
    int a = seed + 1, b = seed + 2, c = seed + 3, d = seed + 4;
    int e = seed + 5, f = seed + 6, g = seed + 7, h = seed + 8;
    
    /* Long sequence of independent computations */
    int t1 = a * b + c - d;
    asm volatile("" : : "r"(t1)); /* Force register use */
    
    int t2 = (e & f) | (g ^ h);
    asm volatile("" : : "r"(t2));
    
    int t3 = t1 * t2 + (a << 2);
    asm volatile("" : : "r"(t3));
    
    /* Function call creates register pressure */
    int t4 = ext_func1(t3);
    
    int t5 = b * c * d * e;
    asm volatile("" : : "r"(t5));
    
    int t6 = (f + g) * (h - a);
    asm volatile("" : : "r"(t6));
    
    int t7 = t4 ^ t5 ^ t6;
    asm volatile("" : : "r"(t7));
    
    int t8 = ext_func2(t7, t1);
    
    int t9 = (t2 << 3) | (t3 >> 2);
    asm volatile("" : : "r"(t9));
    
    int t10 = t8 * t9 + 12345;
    asm volatile("" : : "r"(t10));
    
    /* More computations to increase pressure */
    for (int i = 0; i < 4; i++) {
        t10 = t10 * 2 + i;
        asm volatile("" : : "r"(t10));
    }
    
    result = t10;
    return result;
}

/* Test 2: Mixed integer and floating-point operations */
double __attribute__((noinline)) test_mixed_types(int seed) {
    volatile double result = 0.0;
    int i1 = seed * 2, i2 = seed + 100, i3 = seed - 50;
    double f1 = seed * 1.5, f2 = seed * 2.5, f3 = seed * 0.75;
    
    /* Integer computations */
    int t1 = i1 * i2 + i3;
    asm volatile("" : : "r"(t1));
    
    /* Floating computations */
    double t2 = f1 * f2 + f3;
    asm volatile("" : : "f"(t2));
    
    /* Function call with floating point */
    double t3 = ext_func3(t2);
    
    /* Mixed computation */
    double t4 = t3 + (double)t1;
    asm volatile("" : : "f"(t4));
    
    /* More integer work */
    int t5 = ext_func1(t1);
    asm volatile("" : : "r"(t5));
    
    /* Convert and mix */
    double t6 = t4 * (double)t5;
    asm volatile("" : : "f"(t6));
    
    result = t6;
    return result;
}

/* Test 3: Nested loops with complex induction variables */
long long __attribute__((noinline)) test_nested_loops(int N) {
    volatile long long total = 0;
    
    for (int i = 0; i < N; i++) {
        /* Complex induction variable */
        for (int j = i * 2; j < N; j += 3) {
            /* Inner loop computations creating register pressure */
            int a = i + j;
            int b = i * j;
            int c = a ^ b;
            asm volatile("" : : "r"(c));
            
            int d = ext_func2(a, b);
            asm volatile("" : : "r"(d));
            
            int e = c * d + j;
            asm volatile("" : : "r"(e));
            
            /* Function call inside inner loop */
            long long f = ext_func4(e);
            asm volatile("" : : "r"(f));
            
            total += f;
            
            /* Additional computations */
            for (int k = 0; k < 2; k++) {
                int g = e + k;
                int h = ext_func1(g);
                asm volatile("" : : "r"(h));
                total += h;
            }
        }
    }
    
    return total;
}

/* Test 4: 64-bit and vector operations */
v4si __attribute__((noinline)) test_vector_ops(int seed) {
    v4si v1 = {seed, seed + 1, seed + 2, seed + 3};
    v4si v2 = {seed + 4, seed + 5, seed + 6, seed + 7};
    v4si v3 = {seed + 8, seed + 9, seed + 10, seed + 11};
    
    /* Vector operations */
    v4si t1 = v1 + v2;
    v4si t2 = v1 * v3;
    v4si t3 = t1 & t2;
    
    /* 64-bit operations */
    long long ll1 = (long long)seed * 1000000;
    long long ll2 = ext_func4(ll1);
    asm volatile("" : : "r"(ll2));
    
    long long ll3 = ll1 ^ ll2;
    asm volatile("" : : "r"(ll3));
    
    long long ll4 = ll3 * 7 + 123456;
    asm volatile("" : : "r"(ll4));
    
    /* Mix vector and scalar */
    v4si result = t3 + (v4si){ll4 & 0xFF, (ll4 >> 8) & 0xFF, 
                              (ll4 >> 16) & 0xFF, (ll4 >> 24) & 0xFF};
    
    return result;
}

/* Test 5: Extreme register pressure with many live values */
int __attribute__((noinline)) test_extreme_pressure(int seed) {
    /* Declare many variables to create high register pressure */
    int v1 = seed, v2 = seed * 2, v3 = seed * 3, v4 = seed * 4;
    int v5 = seed * 5, v6 = seed * 6, v7 = seed * 7, v8 = seed * 8;
    int v9 = seed * 9, v10 = seed * 10, v11 = seed * 11, v12 = seed * 12;
    int v13 = seed * 13, v14 = seed * 14, v15 = seed * 15, v16 = seed * 16;
    
    /* Keep all variables live with asm statements */
    asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4));
    asm volatile("" : : "r"(v5), "r"(v6), "r"(v7), "r"(v8));
    asm volatile("" : : "r"(v9), "r"(v10), "r"(v11), "r"(v12));
    asm volatile("" : : "r"(v13), "r"(v14), "r"(v15), "r"(v16));
    
    /* Complex computation using all variables */
    int t1 = v1 * v2 + v3 - v4;
    int t2 = v5 & v6 | v7 ^ v8;
    int t3 = v9 * v10 * v11 * v12;
    int t4 = v13 + v14 + v15 + v16;
    
    asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4));
    
    /* Function calls between computations */
    int t5 = ext_func1(t1);
    int t6 = ext_func2(t2, t3);
    
    /* More computations keeping values live */
    int t7 = t4 * t5 + t6;
    int t8 = (t1 ^ t2) & (t3 | t4);
    
    asm volatile("" : : "r"(t7), "r"(t8));
    
    /* Final computation */
    int result = t7 * t8 + t5 - t6;
    
    /* Use all variables one more time to prevent optimization */
    asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4),
                         "r"(v5), "r"(v6), "r"(v7), "r"(v8),
                         "r"(v9), "r"(v10), "r"(v11), "r"(v12),
                         "r"(v13), "r"(v14), "r"(v15), "r"(v16));
    
    return result;
}

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : time(NULL);
    int N = argc > 2 ? atoi(argv[2]) : 10;
    
    printf("Testing early rematerialization with seed=%d, N=%d\n", seed, N);
    
    /* Run all tests to trigger different parts of the pass */
    int r1 = test_integer_pressure(seed);
    printf("Test 1 result: %d\n", r1);
    
    double r2 = test_mixed_types(seed);
    printf("Test 2 result: %f\n", r2);
    
    long long r3 = test_nested_loops(N);
    printf("Test 3 result: %lld\n", r3);
    
    v4si r4 = test_vector_ops(seed);
    printf("Test 4 result: {%d, %d, %d, %d}\n", 
           r4[0], r4[1], r4[2], r4[3]);
    
    int r5 = test_extreme_pressure(seed);
    printf("Test 5 result: %d\n", r5);
    
    /* Compute checksum to prevent optimization */
    volatile int checksum = r1 + (int)r2 + (int)r3 + r4[0] + r5;
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
