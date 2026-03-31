/* test_early_remat.c - Test program for GCC early rematerialization pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External non-inline functions to force register pressure */
extern int external_func1(int);
extern int external_func2(int, int);
extern double external_func3(double);
extern long long external_func4(long long);

/* Dummy implementations to satisfy linker */
int external_func1(int x) { return x ^ 0x55AA55AA; }
int external_func2(int x, int y) { return x * y + (x ^ y); }
double external_func3(double x) { return x * 1.234567; }
long long external_func4(long long x) { return x * 3 + 1; }

/* Vector type to test different modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High integer register pressure with mixed operations */
int test_integer_pressure(int seed) {
    volatile int result = 0;
    int a = seed * 2 + 1;
    int b = seed / 3 - 5;
    int c = seed ^ 0x12345678;
    int d = seed + 0xABCDEF;
    
    /* Long sequence of independent computations */
    int t1 = a * b + c;
    asm volatile("" : : "r"(t1)); /* Force t1 into register */
    
    int t2 = (c & d) | (a ^ b);
    int t3 = t1 * t2 - a;
    
    /* Function call creates pressure point */
    int t4 = external_func1(t3);
    
    int t5 = (t2 << 3) | (t4 >> 2);
    int t6 = t3 * t4 + t5;
    asm volatile("" : : "r"(t6));
    
    int t7 = external_func2(t5, t6);
    
    int t8 = t6 / (t7 + 1) + t4;
    int t9 = (t8 ^ t7) & 0x7FFFFFFF;
    
    /* More computations to increase pressure */
    int t10 = t9 * 3 - t8;
    int t11 = (t10 << 1) | (t9 >> 1);
    int t12 = external_func1(t11);
    int t13 = t10 + t11 + t12;
    int t14 = t13 * t12 - t11;
    int t15 = (t14 & 0x55555555) | (t13 & 0xAAAAAAAA);
    
    asm volatile("" : : "r"(t15));
    result = t15;
    
    return result;
}

/* Test 2: Mixed integer and floating-point pressure */
double test_mixed_pressure(double seed) {
    volatile double result = 0.0;
    double d1 = seed * 1.234;
    double d2 = seed / 2.345;
    int i1 = (int)(seed * 1000);
    int i2 = (int)(seed * 2000);
    
    /* Mix FP and integer operations */
    double t1 = d1 * d2 + 3.14159;
    asm volatile("" : : "r"(i1), "r"(i2)); /* Force integers into registers */
    
    int t2 = i1 * i2 + (int)d1;
    double t3 = external_func3(t1);
    
    double t4 = t1 * t3 - d2;
    int t5 = external_func1(t2);
    
    double t5_fp = (double)t5 * 0.001;
    double t6 = t4 + t5_fp;
    
    /* More mixed computations */
    int t7 = (int)(t6 * 1000) ^ t5;
    double t8 = external_func3(t6);
    int t9 = external_func2(t7, (int)t8);
    double t10 = t8 * (double)t9;
    
    asm volatile("" : : "r"(t7), "r"(t9));
    result = t10;
    
    return result;
}

/* Test 3: Nested loops with complex induction */
long long test_nested_loops(int N) {
    volatile long long total = 0;
    
    for (int i = 0; i < N; i++) {
        /* Complex induction variable */
        for (int j = i * 2; j < N; j += 3) {
            /* Inner loop with register pressure */
            int a = i * j + 1;
            int b = (i ^ j) * 3;
            int c = external_func1(a);
            int d = external_func2(b, c);
            
            long long t1 = (long long)a * b;
            long long t2 = (long long)c * d;
            long long t3 = external_func4(t1);
            
            /* Force values into registers */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            
            int e = (a & b) | (c ^ d);
            long long t4 = t3 + t2 * e;
            
            total += t4;
            
            /* Additional pressure in loop */
            int f = e * (j % 7) + 1;
            double g = (double)f * 0.12345;
            asm volatile("" : : "r"(f));
            
            total += (long long)(g * 1000);
        }
    }
    
    return total;
}

/* Test 4: Vector operations for vector modes */
v4si test_vector_ops(v4si seed) {
    v4si a = seed + (v4si){1, 2, 3, 4};
    v4si b = seed * (v4si){2, 3, 4, 5};
    v4si c = a & b;
    v4si d = a | b;
    
    /* Create register pressure with vectors */
    v4si t1 = a * b + c;
    v4si t2 = (c & d) | (a ^ b);
    v4si t3 = t1 * t2 - a;
    
    /* Mix with scalar operations */
    int s1 = t1[0] + t1[1];
    int s2 = t2[2] * t2[3];
    asm volatile("" : : "r"(s1), "r"(s2));
    
    v4si t4 = t3 + (v4si){s1, s2, s1, s2};
    v4si t5 = t4 * (v4si){3, 4, 5, 6};
    
    /* Force vector into registers */
    asm volatile("" : : "x"(t5));
    
    return t5;
}

/* Test 5: Extreme register pressure with many live values */
int test_extreme_pressure(int iterations) {
    volatile int checksum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Create many independent live values */
        int v1 = i * 3 + 1;
        int v2 = i * 5 - 2;
        int v3 = i * 7 + 3;
        int v4 = i * 11 - 4;
        int v5 = i * 13 + 5;
        int v6 = i * 17 - 6;
        int v7 = i * 19 + 7;
        int v8 = i * 23 - 8;
        
        /* Keep them all live with asm statements */
        asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4));
        asm volatile("" : : "r"(v5), "r"(v6), "r"(v7), "r"(v8));
        
        /* Complex computation using all values */
        int t1 = v1 * v2 + v3;
        int t2 = v4 ^ v5 | v6;
        int t3 = external_func1(t1);
        int t4 = external_func2(t2, t3);
        
        int t5 = v7 * v8 + t4;
        int t6 = (t3 & t5) | (t4 ^ t1);
        
        asm volatile("" : : "r"(t5), "r"(t6));
        
        checksum += t6;
        
        /* More values to increase pressure */
        int v9 = t6 * 2 + 1;
        int v10 = t5 / 3 - 2;
        double v11 = (double)v9 * 1.234;
        long long v12 = (long long)v10 * 1000LL;
        
        asm volatile("" : : "r"(v9), "r"(v10));
        checksum += (int)v11 + (int)v12;
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    int N = argc > 2 ? atoi(argv[2]) : 100;
    
    printf("Testing early rematerialization coverage...\n");
    
    /* Run all tests to trigger different patterns */
    int r1 = test_integer_pressure(seed);
    printf("Test 1 result: %d\n", r1);
    
    double r2 = test_mixed_pressure((double)seed);
    printf("Test 2 result: %f\n", r2);
    
    long long r3 = test_nested_loops(N);
    printf("Test 3 result: %lld\n", r3);
    
    v4si vec_seed = {seed, seed+1, seed+2, seed+3};
    v4si r4 = test_vector_ops(vec_seed);
    printf("Test 4 result: [%d, %d, %d, %d]\n", r4[0], r4[1], r4[2], r4[3]);
    
    int r5 = test_extreme_pressure(N/10);
    printf("Test 5 result: %d\n", r5);
    
    /* Final volatile store to prevent optimization */
    volatile int final = r1 + (int)r2 + (int)r3 + r4[0] + r5;
    
    return final != 0 ? 0 : 1;
}
