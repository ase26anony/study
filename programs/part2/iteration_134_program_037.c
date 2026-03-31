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
int external_func1(int x) { return x ^ 0x1234; }
int external_func2(int x, int y) { return x * y + 1; }
double external_func3(double x) { return x * 1.5; }
long long external_func4(long long x) { return x + 0xABCDEF; }

/* Vector type to test different modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High integer register pressure with complex expressions */
int test_integer_pressure(int seed) {
    volatile int result = 0;
    int a = seed * 3 + 1;
    int b = seed / 2 - 5;
    int c = seed ^ 0xFF;
    int d = seed + 777;
    
    /* Long sequence of independent computations */
    int t1 = a * b + c;
    asm volatile("" : : "r"(t1)); /* Force t1 to stay in register */
    
    int t2 = (d << 3) | (a & 0xF);
    int t3 = external_func1(t2); /* Function call creates pressure point */
    
    int t4 = t1 ^ t3;
    int t5 = b * c - d;
    asm volatile("" : : "r"(t4), "r"(t5));
    
    int t6 = external_func2(t4, t5);
    int t7 = (t6 >> 4) + a;
    int t8 = t7 * 3 - b;
    
    int t9 = c + d * 2;
    int t10 = external_func1(t9);
    asm volatile("" : : "r"(t8), "r"(t10));
    
    int t11 = t8 & t10;
    int t12 = (t11 << 1) + seed;
    int t13 = external_func2(t11, t12);
    
    /* More computations to increase pressure */
    int t14 = t13 * 7 + 11;
    int t15 = t14 ^ 0xDEADBEEF;
    int t16 = external_func1(t15);
    int t17 = t16 / 3 + t14;
    
    asm volatile("" : : "r"(t17));
    result = t17;
    
    return result;
}

/* Test 2: Mixed integer and floating-point operations */
double test_mixed_types(int seed) {
    volatile double result = 0.0;
    double f1 = seed * 1.234;
    double f2 = seed / 3.14159;
    int i1 = seed * 2;
    int i2 = seed + 1000;
    
    /* Mix FP and integer operations */
    double ft1 = f1 * f2 + 1.5;
    asm volatile("" : : "r"(i1), "r"(i2)); /* Force integers to registers */
    
    int it1 = i1 * i2 - seed;
    double ft2 = external_func3(ft1); /* FP function call */
    
    double ft3 = ft2 / f1;
    int it2 = external_func1(it1); /* Integer function call */
    
    /* More mixed computations */
    double ft4 = ft3 * it2; /* Mixed mode operation */
    int it3 = (int)ft4 ^ i1;
    
    asm volatile("" : : "r"(it3));
    double ft5 = external_func3(ft4);
    
    int it4 = it3 * 3 + i2;
    double ft6 = ft5 + (double)it4;
    
    result = ft6;
    return result;
}

/* Test 3: Nested loops with complex induction */
long long test_nested_loops(int N) {
    volatile long long sum = 0;
    
    for (int i = 0; i < N; i++) {
        /* Complex induction variable */
        for (int j = i * 2; j < N; j += 3) {
            /* Inner loop with register pressure */
            int a = i * j + 1;
            int b = (j << 2) | (i & 0xF);
            asm volatile("" : : "r"(a), "r"(b));
            
            int c = external_func2(a, b);
            long long d = (long long)c * i;
            
            /* More computations in inner loop */
            int e = j * 3 - i;
            long long f = external_func4(d);
            asm volatile("" : : "r"(e));
            
            int g = e ^ 0x55AA;
            long long h = f + g;
            
            sum += h;
            
            /* Function call in inner loop */
            if ((j % 7) == 0) {
                external_func1(j);
            }
        }
        
        /* Additional computation between loops */
        if (i % 5 == 0) {
            int x = i * i - N;
            long long y = external_func4(x);
            sum += y;
        }
    }
    
    return sum;
}

/* Test 4: Vector operations for vector modes */
int test_vector_operations(int seed) {
    v4si v1 = {seed, seed + 1, seed + 2, seed + 3};
    v4si v2 = {seed * 2, seed * 3, seed * 4, seed * 5};
    v4si v3 = {seed + 10, seed + 20, seed + 30, seed + 40};
    
    /* Chain of vector operations */
    v4si r1 = v1 + v2;
    v4si r2 = r1 * v3;
    asm volatile("" : : "r"(r1), "r"(r2));
    
    v4si r3 = r2 - v1;
    v4si r4 = r3 & v2;
    
    /* Scalar operations mixed in */
    int s1 = seed * 7;
    int s2 = external_func1(s1);
    asm volatile("" : : "r"(s2));
    
    v4si r5 = r4 + (v4si){s2, s2, s2, s2};
    v4si r6 = r5 * 2;
    
    /* Extract and compute result */
    int result = 0;
    for (int i = 0; i < 4; i++) {
        result += r6[i];
    }
    
    return result;
}

/* Test 5: Extreme register pressure with many temporaries */
long long test_extreme_pressure(int iterations) {
    volatile long long checksum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Create many unique temporary values */
        long long t1 = i * 123456789LL;
        long long t2 = t1 ^ 0xF0F0F0F0F0F0F0F0LL;
        asm volatile("" : : "r"(t1), "r"(t2));
        
        long long t3 = external_func4(t2);
        long long t4 = t3 + i * 987654321LL;
        
        long long t5 = t4 >> 8;
        long long t6 = t5 * 3;
        asm volatile("" : : "r"(t5), "r"(t6));
        
        long long t7 = external_func4(t6);
        long long t8 = t7 & 0xAAAAAAAAAAAAAAAALL;
        
        long long t9 = t8 + t4;
        long long t10 = t9 * 7 - t2;
        asm volatile("" : : "r"(t9), "r"(t10));
        
        long long t11 = external_func4(t10);
        long long t12 = t11 ^ t8;
        
        /* More temporaries */
        long long t13 = t12 * 13;
        long long t14 = t13 + 0x123456789ABCDEFLL;
        asm volatile("" : : "r"(t13), "r"(t14));
        
        long long t15 = external_func4(t14);
        long long t16 = t15 / 5;
        
        checksum += t16;
        
        /* Periodic function call */
        if (i % 4 == 0) {
            external_func1(i);
        }
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : time(NULL);
    int N = argc > 2 ? atoi(argv[2]) : 100;
    int iterations = argc > 3 ? atoi(argv[3]) : 50;
    
    printf("Testing early rematerialization with seed=%d, N=%d, iterations=%d\n", 
           seed, N, iterations);
    
    /* Run all tests to trigger different parts of the rematerialization pass */
    int r1 = test_integer_pressure(seed);
    printf("Test 1 result: %d\n", r1);
    
    double r2 = test_mixed_types(seed);
    printf("Test 2 result: %f\n", r2);
    
    long long r3 = test_nested_loops(N);
    printf("Test 3 result: %lld\n", r3);
    
    int r4 = test_vector_operations(seed);
    printf("Test 4 result: %d\n", r4);
    
    long long r5 = test_extreme_pressure(iterations);
    printf("Test 5 result: %lld\n", r5);
    
    /* Final checksum to prevent optimization */
    volatile long long final = r1 + (long long)r2 + r3 + r4 + r5;
    printf("Final checksum: %lld\n", final);
    
    return 0;
}
