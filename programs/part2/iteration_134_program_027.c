/* test_early_remat.c - Test program for GCC early rematerialization pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External non-inline functions to force register pressure */
extern int ext_func1(int);
extern double ext_func2(double);
extern long long ext_func3(long long);

/* Dummy external functions */
int ext_func1(int x) { return x ^ 0x55AA55AA; }
double ext_func2(double x) { return x * 1.234567; }
long long ext_func3(long long x) { return x + 0x123456789ABCDEFLL; }

/* Vector type to test different modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High integer register pressure with complex expressions */
int test_integer_pressure(int seed) {
    volatile int result = 0;
    int a = seed * 3 + 1;
    int b = seed / 2 - 5;
    
    /* Long sequence of independent computations */
    int t1 = a * b + seed;
    asm volatile("" : : "r"(t1)); /* Force t1 to be in register */
    
    int t2 = (a & 0xFF) | (b << 8);
    int t3 = t1 ^ t2;
    
    /* Function call creates pressure point */
    int t4 = ext_func1(t3);
    
    int t5 = t4 * 7 - 31;
    asm volatile("" : : "r"(t5));
    
    int t6 = (t5 >> 4) & 0x0F0F0F0F;
    int t7 = t6 + a - b;
    
    int t8 = ext_func1(t7);
    
    int t9 = t8 * 13 + 17;
    int t10 = (t9 & 0x55555555) * 3;
    
    asm volatile("" : : "r"(t10));
    
    int t11 = t10 | (seed << 16);
    int t12 = ext_func1(t11);
    
    int t13 = t12 * 19 - 23;
    int t14 = (t13 ^ 0xAAAAAAAA) + 1;
    
    int t15 = t14 * 29 % 1024;
    asm volatile("" : : "r"(t15));
    
    result = t15;
    return result;
}

/* Test 2: Mixed integer and floating-point pressure */
double test_mixed_pressure(double seed) {
    volatile double result = 0.0;
    double d1 = seed * 1.5;
    double d2 = seed / 3.0;
    
    /* Integer computations */
    int i1 = (int)(seed * 1000);
    int i2 = i1 * 3 + 456;
    asm volatile("" : : "r"(i2));
    
    /* Floating computations */
    double d3 = d1 * d2 + 2.71828;
    double d4 = ext_func2(d3);
    
    int i3 = i2 / 7 - 123;
    double d5 = d4 * 3.14159 - d1;
    
    asm volatile("" : : "r"(i3), "r"(d5));
    
    /* More mixed operations */
    double d6 = d5 + (double)i3;
    int i4 = (int)d6 * 11;
    
    double d7 = ext_func2(d6);
    int i5 = ext_func1(i4);
    
    double d8 = d7 * 2.0 - 1.0;
    int i6 = i5 & 0x7FFFFFFF;
    
    asm volatile("" : : "r"(i6), "r"(d8));
    
    result = d8 + (double)i6;
    return result;
}

/* Test 3: Nested loops with complex induction variables */
long long test_nested_loops(int N) {
    volatile long long total = 0;
    
    for (int i = 0; i < N; i++) {
        /* Complex induction variable for inner loop */
        for (int j = i * 2; j < N; j += 3) {
            /* High register pressure inside loop */
            long long a = i * 1000LL + j;
            long long b = a * 3 - 7;
            
            asm volatile("" : : "r"(b));
            
            long long c = b ^ 0x123456789ABCDEFLL;
            long long d = ext_func3(c);
            
            long long e = d * 5 + 11;
            long long f = (e >> 4) & 0x0F0F0F0F0F0F0F0FLL;
            
            asm volatile("" : : "r"(f));
            
            long long g = f + a - b;
            total += g;
            
            /* Function call increases pressure */
            if ((j % 7) == 0) {
                total += ext_func3(g);
            }
        }
        
        /* Additional computation between outer loop iterations */
        int tmp = ext_func1(i);
        total += tmp;
    }
    
    return total;
}

/* Test 4: 64-bit and vector operations for different modes */
long long test_64bit_vector(int iterations) {
    volatile long long sum = 0;
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    for (int i = 0; i < iterations; i++) {
        /* 64-bit operations */
        long long ll1 = (long long)i * 1000000000LL;
        long long ll2 = ll1 + 0xFFFFFFFFLL;
        
        asm volatile("" : : "r"(ll2));
        
        long long ll3 = ll2 * 3 - 5;
        long long ll4 = ext_func3(ll3);
        
        /* Vector operations */
        v4si vec3 = vec1 + vec2 * i;
        v4si vec4 = vec3 & 0x7F;
        
        /* Mix 64-bit and vector */
        sum += ll4 + vec4[0] + vec4[1] + vec4[2] + vec4[3];
        
        /* Update vectors for next iteration */
        vec1 = vec1 + 1;
        vec2 = vec2 * 2 - 1;
        
        asm volatile("" : : "r"(vec1), "r"(vec2));
        
        /* Function call */
        if (i % 3 == 0) {
            sum += ext_func1(i);
        }
    }
    
    return sum;
}

/* Test 5: Extreme register pressure with many live values */
int test_extreme_pressure(int seed) {
    volatile int final = 0;
    
    /* Create many independent live values */
    int v1 = seed + 1;
    int v2 = seed * 2 - 3;
    int v3 = v1 ^ v2;
    int v4 = ext_func1(v3);
    
    int v5 = v4 * 5 + 7;
    int v6 = (v5 >> 2) & 0x3F;
    asm volatile("" : : "r"(v6));
    
    int v7 = v6 | (seed << 8);
    int v8 = ext_func1(v7);
    
    int v9 = v8 * 11 - 13;
    int v10 = v9 & 0xFF00FF;
    asm volatile("" : : "r"(v10));
    
    int v11 = v10 + v1 - v2;
    int v12 = ext_func1(v11);
    
    int v13 = v12 * 17 + 19;
    int v14 = (v13 ^ 0xCCCCCCCC) >> 1;
    
    int v15 = v14 * 23 % 256;
    int v16 = ext_func1(v15);
    
    int v17 = v16 * 29 - 31;
    int v18 = v17 & 0x0F0F0F0F;
    asm volatile("" : : "r"(v18));
    
    int v19 = v18 + v3 - v4;
    int v20 = ext_func1(v19);
    
    /* Use all values in final computation */
    final = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
            v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    
    return final;
}

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    int N = argc > 2 ? atoi(argv[2]) : 100;
    
    printf("Testing early rematerialization coverage...\n");
    
    /* Run all tests to maximize coverage opportunities */
    int r1 = test_integer_pressure(seed);
    printf("Test 1 result: %d\n", r1);
    
    double r2 = test_mixed_pressure((double)seed);
    printf("Test 2 result: %f\n", r2);
    
    long long r3 = test_nested_loops(N);
    printf("Test 3 result: %lld\n", r3);
    
    long long r4 = test_64bit_vector(N / 10);
    printf("Test 4 result: %lld\n", r4);
    
    int r5 = test_extreme_pressure(seed);
    printf("Test 5 result: %d\n", r5);
    
    /* Final volatile store to prevent optimization */
    volatile int checksum = r1 + (int)r2 + (int)r3 + (int)r4 + r5;
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
