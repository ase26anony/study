/* test_early_remat.c - Test program for GCC early rematerialization pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External non-inline functions to force register pressure */
extern int ext_func1(int);
extern int ext_func2(int, int);
extern double ext_func3(double);
extern long long ext_func4(long long);

/* Dummy external functions */
int ext_func1(int x) { return x ^ 0x55AA55AA; }
int ext_func2(int x, int y) { return (x * y) ^ 0x12345678; }
double ext_func3(double x) { return x * 1.234567; }
long long ext_func4(long long x) { return x + 0xFEDCBA9876543210LL; }

/* Vector type to test different modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High integer register pressure with complex expressions */
int test_integer_pressure(int seed) {
    volatile int result = 0;
    int a = seed + 1;
    int b = seed * 2;
    int c = seed / 3;
    int d = seed ^ 0x1234;
    int e = seed | 0xABCD;
    
    /* Long sequence of independent computations */
    int t1 = a * b + c;
    asm volatile("" : : "r"(t1)); /* Pin in register */
    
    int t2 = (d << 3) | (e >> 2);
    asm volatile("" : : "r"(t2));
    
    int t3 = ext_func1(t1); /* Function call creates pressure */
    
    int t4 = t2 ^ t3;
    int t5 = t4 * 0x9E3779B9;
    asm volatile("" : : "r"(t5));
    
    int t6 = ext_func2(t3, t5);
    
    int t7 = (t4 + t5) * (t6 - t1);
    int t8 = t7 & 0x7FFFFFFF;
    asm volatile("" : : "r"(t8));
    
    int t9 = t8 | (t2 << 16);
    int t10 = t9 ^ t6;
    
    int t11 = ext_func1(t10);
    
    int t12 = t11 * 31 + 17;
    int t13 = (t12 >> 5) & 0xFF;
    asm volatile("" : : "r"(t13));
    
    int t14 = t13 * t10;
    int t15 = t14 - t9;
    
    result = t15;
    return result;
}

/* Test 2: Mixed integer and floating-point pressure */
double test_mixed_pressure(int seed) {
    volatile double result = 0.0;
    double d1 = seed * 1.234;
    double d2 = seed / 5.678;
    float f1 = seed * 0.123f;
    float f2 = seed + 456.789f;
    
    int i1 = seed * 3;
    int i2 = seed + 1000;
    
    /* Mix operations */
    double t1 = d1 * d2 + f1 * f2;
    asm volatile("" : : "r"(i1), "r"(i2)); /* Pin integers */
    
    float t2 = f1 * 2.5f - f2 / 1.5f;
    asm volatile("" : : "f"(t2)); /* Pin float */
    
    int t3 = ext_func2(i1, i2); /* Function call */
    
    double t4 = ext_func3(t1);
    
    int t5 = t3 ^ (int)t4;
    asm volatile("" : : "r"(t5));
    
    double t6 = t4 * 3.14159 + d1;
    float t7 = (float)t6 * f1;
    
    int t8 = ext_func1(t5);
    
    double t9 = t6 / t4 - t1;
    asm volatile("" : : "f"(t7), "f"(t9)); /* Pin floats */
    
    int t10 = t8 * 7 + t5;
    double t11 = t9 * 2.71828;
    
    result = t11 + t7 + t10;
    return result;
}

/* Test 3: Nested loops with complex induction */
long long test_nested_loops(int N) {
    volatile long long sum = 0;
    
    for (int i = 0; i < N; i++) {
        /* Complex induction variable */
        for (int j = i * 2; j < N; j += 3) {
            int a = i * j + 1;
            int b = (i ^ j) * 3;
            asm volatile("" : : "r"(a), "r"(b));
            
            int c = ext_func1(a);
            int d = ext_func2(b, c);
            
            /* More computations in loop body */
            int e = (a << 4) | (b >> 2);
            int f = d * 7 - e;
            asm volatile("" : : "r"(e), "r"(f));
            
            int g = ext_func1(f);
            
            /* Use all computed values */
            sum += (long long)a * b + c - d + e * f + g;
            
            /* Function call in inner loop increases pressure */
            if ((j % 7) == 0) {
                sum += ext_func4(sum);
            }
        }
        
        /* Additional computation between loops */
        int x = ext_func2(i, N);
        sum += x * 11LL;
    }
    
    return sum;
}

/* Test 4: Vector operations for vector modes */
v4si test_vector_ops(int seed) {
    v4si v1 = {seed, seed + 1, seed + 2, seed + 3};
    v4si v2 = {seed * 2, seed * 3, seed * 4, seed * 5};
    v4si v3 = {seed ^ 0xAA, seed ^ 0xBB, seed ^ 0xCC, seed ^ 0xDD};
    
    /* Chain of vector operations */
    v4si r1 = v1 + v2;
    v4si r2 = v1 * v3;
    asm volatile("" : : "x"(r1), "x"(r2)); /* Pin vectors */
    
    v4si r3 = r1 & v2;
    v4si r4 = r2 | v3;
    
    /* Scalar operations mixed in */
    int s1 = seed * 17;
    int s2 = ext_func1(s1);
    asm volatile("" : : "r"(s1), "r"(s2));
    
    v4si r5 = r3 + r4;
    v4si r6 = r5 * v1;
    
    int s3 = ext_func2(s1, s2);
    v4si mask = {s3, s3 ^ 1, s3 ^ 2, s3 ^ 3};
    
    v4si r7 = r6 & mask;
    asm volatile("" : : "x"(r7));
    
    return r7;
}

/* Test 5: Extreme register pressure with many live values */
long long test_extreme_pressure(int iterations) {
    volatile long long checksum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Create many independent live values */
        long long ll1 = i * 1000000007LL;
        long long ll2 = i * 998244353LL;
        long long ll3 = ll1 ^ ll2;
        double d1 = i * 3.1415926535;
        double d2 = i * 2.7182818284;
        int i1 = i * 31;
        int i2 = i * 127;
        int i3 = i1 ^ i2;
        int i4 = i1 | i2;
        
        /* Pin all values */
        asm volatile("" : : "r"(ll1), "r"(ll2), "r"(ll3),
                     "r"(i1), "r"(i2), "r"(i3), "r"(i4));
        asm volatile("" : : "f"(d1), "f"(d2));
        
        /* Function calls that use different value types */
        int r1 = ext_func1(i3);
        double r2 = ext_func3(d1);
        long long r3 = ext_func4(ll3);
        
        /* More computations creating pressure */
        int i5 = r1 * i4;
        double d3 = r2 * d2;
        long long ll4 = r3 + ll1;
        
        asm volatile("" : : "r"(i5), "f"(d3), "r"(ll4));
        
        int r4 = ext_func2(i5, (int)ll4);
        double r5 = ext_func3(d3);
        
        /* Final computation using all values */
        checksum += (long long)r1 + (long long)r4 + (long long)(r2 + r5) + ll4;
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    int N = argc > 2 ? atoi(argv[2]) : 100;
    int iterations = argc > 3 ? atoi(argv[3]) : 50;
    
    printf("Testing early rematerialization coverage...\n");
    
    /* Call all test functions to ensure they're not optimized away */
    int r1 = test_integer_pressure(seed);
    printf("Test 1 result: %d\n", r1);
    
    double r2 = test_mixed_pressure(seed);
    printf("Test 2 result: %f\n", r2);
    
    long long r3 = test_nested_loops(N);
    printf("Test 3 result: %lld\n", r3);
    
    v4si r4 = test_vector_ops(seed);
    printf("Test 4 result: %d %d %d %d\n", r4[0], r4[1], r4[2], r4[3]);
    
    long long r5 = test_extreme_pressure(iterations);
    printf("Test 5 result: %lld\n", r5);
    
    /* Use results in volatile store to prevent elimination */
    volatile int dummy = r1 + (int)r2 + (int)r3 + r4[0] + (int)r5;
    
    return 0;
}
