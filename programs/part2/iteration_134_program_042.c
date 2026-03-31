/* test_early_remat.c - Test program for GCC early rematerialization pass */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External non-inline functions to force register pressure */
extern int external_func1(int);
extern int external_func2(int, int);
extern double external_func3(double);
extern long long external_func4(long long);

/* Dummy external functions */
int external_func1(int x) { return x ^ 0x55AA55AA; }
int external_func2(int x, int y) { return x * y + (x ^ y); }
double external_func3(double x) { return x * 1.23456789; }
long long external_func4(long long x) { return x * 3LL; }

/* Vector type to test different modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High integer register pressure with complex expressions */
int test_integer_pressure(int seed) {
    volatile int result = 0;
    int a = seed + 1;
    int b = seed * 2;
    int c = seed / 3;
    int d = seed ^ 0x12345678;
    int e = seed | 0x87654321;
    int f = seed & 0xF0F0F0F0;
    int g = seed << 3;
    int h = seed >> 2;
    
    /* Force values into registers with inline asm */
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
    
    /* Long sequence of independent computations */
    int t1 = a * b + c;
    int t2 = d & e | f;
    int t3 = g ^ h + seed;
    int t4 = t1 * t2 - t3;
    int t5 = t2 / (t1 + 1) | t4;
    int t6 = t3 ^ t4 & t5;
    int t7 = t4 * t5 + t6;
    int t8 = t5 | t6 ^ t7;
    int t9 = t6 & t7 | t8;
    int t10 = t7 * t8 - t9;
    
    /* Force intermediate values */
    asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4), "r"(t5));
    
    /* Function call that clobbers registers */
    int r1 = external_func1(t1);
    int r2 = external_func2(t2, t3);
    
    /* More computations after call */
    int t11 = t8 * r1 + t9;
    int t12 = t9 & r2 | t10;
    int t13 = t10 ^ t11 + t12;
    int t14 = t11 * t12 - t13;
    int t15 = t12 / (t13 + 1) | t14;
    
    asm volatile("" : : "r"(t6), "r"(t7), "r"(t8), "r"(t9), "r"(t10));
    
    /* Another function call */
    int r3 = external_func1(t11);
    
    /* Final computation */
    result = t13 + t14 + t15 + r1 + r2 + r3;
    
    /* Use result to prevent elimination */
    asm volatile("" : : "r"(result));
    return result;
}

/* Test 2: Mixed integer and floating-point pressure */
double test_mixed_pressure(int seed) {
    volatile double result = 0.0;
    int i1 = seed;
    int i2 = seed * 2;
    double f1 = seed * 1.5;
    double f2 = seed * 2.5;
    long long ll1 = seed * 1000LL;
    long long ll2 = seed * 2000LL;
    
    /* Force different types into registers */
    asm volatile("" : : "r"(i1), "r"(i2), "r"(ll1), "r"(ll2));
    asm volatile("" : : "f"(f1), "f"(f2));
    
    /* Mixed computations */
    int t1 = i1 * i2 + seed;
    double t2 = f1 * f2 + seed;
    long long t3 = ll1 * ll2 / 1000LL;
    double t4 = t2 * external_func3(f1);
    int t5 = external_func1(t1);
    long long t6 = external_func4(t3);
    
    asm volatile("" : : "r"(t1), "r"(t5), "r"(t3), "r"(t6));
    asm volatile("" : : "f"(t2), "f"(t4));
    
    /* More mixed operations */
    double t7 = t2 + (double)t1;
    int t8 = t5 + (int)t4;
    long long t9 = t6 * (long long)t8;
    double t10 = t4 * (double)t9;
    
    /* Function call between dependent computations */
    double r1 = external_func3(t7);
    int r2 = external_func2(t8, t5);
    
    /* Final mixed computation */
    result = t10 + r1 + (double)r2 + (double)t9;
    
    asm volatile("" : : "f"(result));
    return result;
}

/* Test 3: Nested loops with complex induction variables */
int test_nested_loops(int N) {
    volatile int sum = 0;
    
    for (int i = 0; i < N; i++) {
        /* Complex induction variable for inner loop */
        for (int j = i * 2; j < N; j += 3) {
            int a = i * j;
            int b = j * j - i;
            int c = a ^ b;
            int d = (a & b) | c;
            
            /* Force values in inner loop */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            
            /* Function call in inner loop */
            int r = external_func1(d);
            
            /* More computations */
            int e = a * r + b;
            int f = c & d | e;
            int g = external_func2(e, f);
            
            sum += g;
            
            /* Force more register pressure */
            asm volatile("" : : "r"(e), "r"(f), "r"(g));
        }
        
        /* Additional computation between outer loop iterations */
        int x = i * i + sum;
        int y = external_func1(x);
        sum += y;
        
        asm volatile("" : : "r"(x), "r"(y));
    }
    
    return sum;
}

/* Test 4: Vector operations for vector modes */
int test_vector_operations(int seed) {
    v4si v1 = {seed, seed + 1, seed + 2, seed + 3};
    v4si v2 = {seed * 2, seed * 3, seed * 4, seed * 5};
    v4si v3 = {seed ^ 1, seed ^ 2, seed ^ 3, seed ^ 4};
    
    /* Vector operations */
    v4si r1 = v1 + v2;
    v4si r2 = v1 * v2;
    v4si r3 = r1 & v3;
    v4si r4 = r2 | r3;
    
    /* Extract and use scalar elements to prevent elimination */
    int e1 = r1[0] + r1[1];
    int e2 = r2[2] + r2[3];
    int e3 = r3[0] ^ r3[2];
    int e4 = r4[1] & r4[3];
    
    /* Force vector values (compiler may use multiple registers) */
    asm volatile("" : : "x"(r1), "x"(r2), "x"(r3), "x"(r4));
    
    /* Mixed with scalar computations */
    int s1 = external_func1(e1);
    int s2 = external_func2(e2, e3);
    int s3 = s1 * s2 + e4;
    
    /* More vector operations */
    v4si v4 = {s1, s2, s3, seed};
    v4si r5 = r4 * v4;
    v4si r6 = r5 + r1;
    
    /* Final result */
    int result = r6[0] + r6[1] + r6[2] + r6[3];
    
    asm volatile("" : : "r"(result));
    return result;
}

/* Test 5: Extreme register pressure with many live values */
long long test_extreme_pressure(int seed) {
    /* Declare many variables to increase pressure */
    long long v1 = seed;
    long long v2 = seed * 2LL;
    long long v3 = seed * 3LL;
    long long v4 = seed * 4LL;
    long long v5 = seed * 5LL;
    long long v6 = seed * 6LL;
    long long v7 = seed * 7LL;
    long long v8 = seed * 8LL;
    long long v9 = seed * 9LL;
    long long v10 = seed * 10LL;
    
    /* Force all into registers */
    asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4), "r"(v5),
                       "r"(v6), "r"(v7), "r"(v8), "r"(v9), "r"(v10));
    
    /* Chain of dependent computations */
    long long t1 = v1 * v2 + v3;
    long long t2 = v4 & v5 | v6;
    long long t3 = v7 ^ v8 + v9;
    long long t4 = t1 * t2 - t3;
    long long t5 = t2 / (t1 + 1LL) | t4;
    long long t6 = t3 ^ t4 & t5;
    long long t7 = t4 * t5 + t6;
    long long t8 = t5 | t6 ^ t7;
    long long t9 = t6 & t7 | t8;
    long long t10 = t7 * t8 - t9;
    
    /* Force intermediates */
    asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4), "r"(t5),
                       "r"(t6), "r"(t7), "r"(t8), "r"(t9), "r"(t10));
    
    /* Multiple function calls */
    long long r1 = external_func4(t1);
    long long r2 = external_func4(t2);
    long long r3 = external_func4(t3);
    
    /* More computations with all values live */
    long long f1 = t4 + r1;
    long long f2 = t5 * r2;
    long long f3 = t6 & r3;
    long long f4 = f1 | f2 ^ f3;
    long long f5 = f2 * f3 + f4;
    
    /* Final result using many values */
    long long result = f1 + f2 + f3 + f4 + f5 + r1 + r2 + r3 +
                      t7 + t8 + t9 + t10;
    
    asm volatile("" : : "r"(result));
    return result;
}

int main(int argc, char **argv) {
    int seed = argc > 1 ? atoi(argv[1]) : time(NULL);
    
    printf("Testing early rematerialization coverage...\n");
    
    /* Run all tests to exercise different patterns */
    int r1 = test_integer_pressure(seed);
    double r2 = test_mixed_pressure(seed);
    int r3 = test_nested_loops(seed % 50 + 10);
    int r4 = test_vector_operations(seed);
    long long r5 = test_extreme_pressure(seed);
    
    /* Use results to prevent elimination */
    volatile int checksum = r1 + (int)r2 + r3 + r4 + (int)r5;
    
    printf("Results: %d, %.2f, %d, %d, %lld\n", r1, r2, r3, r4, r5);
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
