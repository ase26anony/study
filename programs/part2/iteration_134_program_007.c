/* test_early_remat.c - Test program for GCC early rematerialization pass coverage */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External non-inline functions to force register pressure */
extern int external_func1(int);
extern int external_func2(int, int);
extern double external_func3(double);
extern long long external_func4(long long);

/* Dummy external functions to prevent inlining */
int external_func1(int x) { return x ^ 0x55AA55AA; }
int external_func2(int x, int y) { return (x * y) ^ 0x12345678; }
double external_func3(double x) { return x * 3.141592653589793; }
long long external_func4(long long x) { return x * 6364136223846793005ULL; }

/* Vector type to engage different machine modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High register pressure with mixed integer operations */
int test_high_pressure_integer(int seed) {
    volatile int result = 0;
    int a = seed + 1;
    int b = seed * 2;
    int c = seed ^ 0xDEADBEEF;
    int d = seed - 12345;
    
    /* Long sequence of independent computations */
    int t1 = a * b + c;
    asm volatile("" : : "r"(t1)); /* Force t1 into register */
    
    int t2 = (b & c) | (d ^ a);
    asm volatile("" : : "r"(t2));
    
    int t3 = external_func1(t1); /* Function call creates pressure point */
    
    int t4 = t2 * t3 - a;
    asm volatile("" : : "r"(t4));
    
    int t5 = (t1 ^ t2) & (t3 | t4);
    asm volatile("" : : "r"(t5));
    
    int t6 = external_func2(t4, t5);
    
    int t7 = t3 + t5 * t6;
    asm volatile("" : : "r"(t7));
    
    int t8 = (t6 << 3) | (t7 >> 2);
    asm volatile("" : : "r"(t8));
    
    int t9 = t4 - t7 + t8;
    asm volatile("" : : "r"(t9));
    
    int t10 = external_func1(t9);
    
    /* More computations to increase pressure */
    int t11 = t1 * t6 + t9;
    int t12 = t2 & t7 | t10;
    int t13 = t3 ^ t8 - t11;
    int t14 = t4 | t9 & t12;
    int t15 = t5 * t10 + t13;
    int t16 = t6 - t11 ^ t14;
    int t17 = t7 & t12 | t15;
    int t18 = t8 ^ t13 - t16;
    int t19 = t9 | t14 & t17;
    int t20 = t10 * t15 + t18;
    
    /* Use all temporaries in final computation */
    result = t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
             t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20;
    
    return result;
}

/* Test 2: Mixed floating-point and integer operations */
double test_mixed_types(int seed) {
    volatile double result = 0.0;
    double f1 = seed * 1.234;
    double f2 = seed / 456.789;
    int i1 = seed ^ 0xABCD;
    int i2 = seed * 3;
    
    /* Mix float and int operations */
    double ft1 = f1 * f2 + i1;
    asm volatile("" : : "r"(ft1));
    
    int it1 = i1 * i2 + (int)f1;
    asm volatile("" : : "r"(it1));
    
    double ft2 = external_func3(ft1); /* Function call */
    
    int it2 = external_func1(it1);
    
    double ft3 = ft1 * ft2 - i2;
    asm volatile("" : : "r"(ft3));
    
    int it3 = (it1 & it2) | (i1 ^ i2);
    asm volatile("" : : "r"(it3));
    
    double ft4 = ft2 / ft3 + it3;
    asm volatile("" : : "r"(ft4));
    
    int it4 = it2 * it3 - (int)ft3;
    asm volatile("" : : "r"(it4));
    
    /* More mixed computations */
    long long ll1 = (long long)seed * 1000000007LL;
    long long ll2 = external_func4(ll1);
    
    double ft5 = ft3 * ft4 + ll1;
    asm volatile("" : : "r"(ft5));
    
    long long ll3 = ll1 ^ ll2 + it4;
    asm volatile("" : : "r"(ll3));
    
    result = ft1 + ft2 + ft3 + ft4 + ft5 + i1 + i2 + it1 + it2 + it3 + it4 + ll1 + ll2 + ll3;
    
    return result;
}

/* Test 3: Nested loops with complex induction variables */
int test_nested_loops(int N) {
    volatile int sum = 0;
    
    for (int i = 0; i < N; i++) {
        /* Non-trivial induction variable for inner loop */
        for (int j = i * 2; j < N; j += 3) {
            /* Complex expression using both i and j */
            int val = (i * j) ^ (i + j) & (i - j);
            asm volatile("" : : "r"(val)); /* Force register use */
            
            /* Additional computations inside loop */
            int t1 = val * 3 + i;
            int t2 = (j << 2) | (i >> 1);
            int t3 = external_func1(t1); /* Pressure point */
            
            int t4 = t1 ^ t2 & t3;
            int t5 = (val + t4) * (j - i);
            
            sum += t5;
            
            /* More register pressure */
            int t6 = t2 * t3 - t4;
            int t7 = (t5 & 0xFF) | (t6 << 8);
            int t8 = external_func2(t6, t7);
            
            sum += t8;
        }
        
        /* Additional computation between inner loops */
        int outer_temp = i * i - i;
        asm volatile("" : : "r"(outer_temp));
        
        if (i % 7 == 0) {
            int special = external_func1(outer_temp);
            sum += special;
        }
    }
    
    return sum;
}

/* Test 4: Vector operations to engage vector modes */
int test_vector_operations(int seed) {
    v4si v1 = {seed, seed + 1, seed + 2, seed + 3};
    v4si v2 = {seed * 2, seed * 3, seed * 4, seed * 5};
    v4si v3 = {seed ^ 1, seed ^ 2, seed ^ 3, seed ^ 4};
    
    /* Vector operations */
    v4si r1 = v1 + v2;
    v4si r2 = v1 * v3;
    v4si r3 = r1 & v2;
    v4si r4 = r2 | v3;
    
    /* Scalar operations mixed with vector */
    int s1 = seed * 100;
    int s2 = external_func1(s1);
    
    v4si r5 = r3 + r4;
    v4si r6 = r5 * (v4si){s1, s2, s1 ^ s2, s1 & s2};
    
    /* Extract and compute */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        int elem = r6[i];
        asm volatile("" : : "r"(elem));
        sum += elem * (i + 1);
        
        /* Additional pressure */
        int temp = external_func2(elem, i);
        sum += temp;
    }
    
    return sum;
}

/* Test 5: Extreme register pressure with many live values */
long long test_extreme_pressure(int iterations) {
    volatile long long checksum = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Create many independent live values */
        long long v1 = iter * 123456789;
        long long v2 = iter ^ 0xF0F0F0F0;
        long long v3 = v1 + v2;
        long long v4 = v1 * v2;
        long long v5 = v3 ^ v4;
        long long v6 = external_func4(v5);
        long long v7 = v2 - v3;
        long long v8 = v4 | v5;
        long long v9 = v6 & v7;
        long long v10 = v8 ^ v9;
        
        /* Force all into registers */
        asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4), "r"(v5));
        asm volatile("" : : "r"(v6), "r"(v7), "r"(v8), "r"(v9), "r"(v10));
        
        /* Function call that clobbers registers */
        long long v11 = external_func4(v10);
        
        /* More computations */
        long long v12 = v1 * v11 + v2;
        long long v13 = v3 & v12 | v4;
        long long v14 = v5 ^ v13 - v6;
        long long v15 = v7 | v14 & v8;
        long long v16 = v9 * v15 + v10;
        long long v17 = v11 - v16 ^ v12;
        long long v18 = v13 & v17 | v14;
        long long v19 = v15 ^ v18 - v16;
        long long v20 = v17 | v19 & v18;
        
        checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                    v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
        
        /* Additional pressure every few iterations */
        if (iter % 5 == 0) {
            long long extra = external_func4(checksum);
            checksum ^= extra;
        }
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    int N = argc > 2 ? atoi(argv[2]) : 100;
    
    printf("Testing early rematerialization coverage...\n");
    
    /* Run all tests to exercise different patterns */
    int r1 = test_high_pressure_integer(seed);
    printf("Test 1 result: %d\n", r1);
    
    double r2 = test_mixed_types(seed);
    printf("Test 2 result: %f\n", r2);
    
    int r3 = test_nested_loops(N);
    printf("Test 3 result: %d\n", r3);
    
    int r4 = test_vector_operations(seed);
    printf("Test 4 result: %d\n", r4);
    
    long long r5 = test_extreme_pressure(N / 10);
    printf("Test 5 result: %lld\n", r5);
    
    /* Final volatile store to prevent optimization */
    volatile int final_result = r1 + (int)r2 + r3 + r4 + (int)r5;
    printf("Final checksum: %d\n", final_result);
    
    return 0;
}
