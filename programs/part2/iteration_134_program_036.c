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
int external_func2(int x, int y) { return (x * y) ^ 0x12345678; }
double external_func3(double x) { return x * 1.234567; }
long long external_func4(long long x) { return x * 3LL; }

/* Vector type to test different modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High integer register pressure with complex expressions */
int test_integer_pressure(int seed) {
    volatile int result = 0;
    int a = seed * 3 + 1;
    int b = seed / 2 - 5;
    
    /* Long sequence of independent computations */
    int t1 = a * b + (seed << 3);
    asm volatile("" : : "r"(t1)); /* Force t1 to be in register */
    
    int t2 = (a & 0xFF) | (b << 8);
    int t3 = t1 ^ t2 ^ seed;
    asm volatile("" : : "r"(t3));
    
    /* Function call creates pressure point */
    int t4 = external_func1(t3);
    
    int t5 = t4 * 7 - t2;
    int t6 = (t5 >> 4) & 0x0F0F0F0F;
    asm volatile("" : : "r"(t6));
    
    int t7 = external_func2(t6, t4);
    
    int t8 = t7 + t5 * 3;
    int t9 = (t8 << 1) | (t8 >> 31);
    asm volatile("" : : "r"(t9));
    
    int t10 = t9 - t6 * 5;
    int t11 = (t10 & 0x55555555) * 3;
    asm volatile("" : : "r"(t11));
    
    int t12 = external_func1(t11);
    
    int t13 = t12 ^ t9 ^ t4;
    int t14 = t13 * 13 + t11;
    asm volatile("" : : "r"(t14));
    
    int t15 = (t14 << 3) | (t14 >> 29);
    int t16 = t15 - t12 + t7;
    asm volatile("" : : "r"(t16));
    
    result = t16;
    return result;
}

/* Test 2: Mixed integer and floating-point pressure */
double test_mixed_pressure(int seed) {
    volatile double result = 0.0;
    double d1 = seed * 1.234;
    double d2 = seed / 3.456;
    
    /* Integer computations */
    int i1 = seed * 3;
    int i2 = i1 + 7;
    asm volatile("" : : "r"(i2));
    
    /* Floating computations */
    double d3 = d1 * d2 + 1.0;
    double d4 = d3 / 2.0 - d1;
    asm volatile("" : : "r"(d4));
    
    /* Function call with floating point */
    double d5 = external_func3(d4);
    
    /* More mixed computations */
    int i3 = i2 * 5 + (int)d5;
    double d6 = d5 * i3;
    asm volatile("" : : "r"(d6), "r"(i3));
    
    int i4 = external_func1(i3);
    double d7 = external_func3(d6);
    
    /* Complex expression mixing types */
    double d8 = d7 * (i4 + 1) / (d6 + 1.0);
    int i5 = (int)d8 * i4;
    asm volatile("" : : "r"(d8), "r"(i5));
    
    result = d8 + i5;
    return result;
}

/* Test 3: Nested loops with complex induction */
long long test_nested_loops(int N) {
    volatile long long total = 0;
    
    for (int i = 0; i < N; i++) {
        /* Non-trivial induction variable */
        for (int j = i * 2; j < N; j += 3) {
            /* Complex expression in inner loop */
            int a = i * 3 + 1;
            int b = j * 5 - 2;
            asm volatile("" : : "r"(a), "r"(b));
            
            int c = a * b + (i ^ j);
            int d = (c << 4) | (c >> 28);
            asm volatile("" : : "r"(d));
            
            /* Function call inside loop */
            int e = external_func2(d, c);
            
            long long f = (long long)e * (i + 1) * (j + 1);
            asm volatile("" : : "r"(f));
            
            total += f;
            
            /* More computations to increase pressure */
            int g = external_func1(e);
            long long h = external_func4(f);
            asm volatile("" : : "r"(g), "r"(h));
            
            total += h - g;
        }
    }
    
    return total;
}

/* Test 4: 64-bit and vector operations */
long long test_64bit_vector(int seed) {
    volatile long long result = 0;
    
    /* 64-bit operations */
    long long ll1 = (long long)seed * 1000000007LL;
    long long ll2 = ll1 << 5;
    long long ll3 = ll2 ^ 0x123456789ABCDEFLL;
    asm volatile("" : : "r"(ll3));
    
    long long ll4 = external_func4(ll3);
    
    /* Vector operations */
    v4si v1 = {seed, seed + 1, seed + 2, seed + 3};
    v4si v2 = {5, 6, 7, 8};
    v4si v3 = v1 * v2;
    v4si v4 = v3 + v1;
    asm volatile("" : : "r"(v4));
    
    /* Mix 64-bit and vector */
    long long ll5 = ll4 * (v4[0] + v4[1] + v4[2] + v4[3]);
    asm volatile("" : : "r"(ll5));
    
    /* More complex chain */
    long long ll6 = external_func4(ll5);
    int i1 = external_func1((int)ll6);
    double d1 = external_func3((double)i1);
    asm volatile("" : : "r"(i1), "r"(d1));
    
    long long ll7 = ll6 * (long long)d1 * (long long)i1;
    result = ll7;
    
    return result;
}

/* Test 5: Extreme register pressure with many live values */
int test_extreme_pressure(int iterations) {
    volatile int checksum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Create many independent live values */
        int v1 = i * 3 + 1;
        int v2 = i * 5 - 2;
        int v3 = v1 ^ v2;
        int v4 = v1 * v2 + v3;
        int v5 = (v4 << 3) | (v4 >> 29);
        int v6 = external_func1(v5);
        
        asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4), "r"(v5), "r"(v6));
        
        int v7 = v6 * 7 + v4;
        int v8 = v7 & 0x0F0F0F0F;
        int v9 = v8 ^ v5 ^ v2;
        int v10 = external_func2(v9, v7);
        
        asm volatile("" : : "r"(v7), "r"(v8), "r"(v9), "r"(v10));
        
        int v11 = v10 * 11 - v8;
        int v12 = (v11 >> 4) + v9;
        int v13 = v12 * 13;
        int v14 = external_func1(v13);
        
        asm volatile("" : : "r"(v11), "r"(v12), "r"(v13), "r"(v14));
        
        checksum += v14;
        
        /* Function call to force register clobbering */
        if (i % 3 == 0) {
            external_func2(v14, checksum);
        }
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : time(NULL);
    int N = argc > 2 ? atoi(argv[2]) : 100;
    
    printf("Testing early rematerialization coverage...\n");
    
    /* Run all tests to trigger different parts of the pass */
    int r1 = test_integer_pressure(seed);
    printf("Test 1 result: %d\n", r1);
    
    double r2 = test_mixed_pressure(seed);
    printf("Test 2 result: %f\n", r2);
    
    long long r3 = test_nested_loops(N);
    printf("Test 3 result: %lld\n", r3);
    
    long long r4 = test_64bit_vector(seed);
    printf("Test 4 result: %lld\n", r4);
    
    int r5 = test_extreme_pressure(N);
    printf("Test 5 result: %d\n", r5);
    
    /* Final volatile store to prevent optimization */
    volatile int final = r1 + (int)r2 + (int)r3 + (int)r4 + r5;
    printf("Final checksum: %d\n", final);
    
    return 0;
}
