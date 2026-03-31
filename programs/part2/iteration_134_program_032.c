/* test_early_remat.c - Test program for GCC early rematerialization pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External non-inline functions to create pressure points */
extern int ext_func1(int);
extern double ext_func2(double);
extern long long ext_func3(long long);

/* Vector type for additional mode coverage */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High register pressure with mixed integer operations */
int test_mixed_integer_pressure(int seed) {
    volatile int result = 0;
    int a = seed * 3 + 1;
    int b = seed / 2 - 5;
    
    /* Long sequence of independent computations */
    int t1 = a * b + seed;
    asm volatile("" : : "r"(t1)); /* Pin in register */
    
    int t2 = (a & b) | (seed << 2);
    int t3 = t1 ^ t2;
    asm volatile("" : : "r"(t3));
    
    int t4 = t2 * 7 - t3;
    int t5 = (t4 >> 3) + t1;
    
    /* Function call creates pressure point */
    int t6 = ext_func1(t5);
    
    int t7 = t6 * a + b;
    int t8 = t7 & 0xFF;
    asm volatile("" : : "r"(t8));
    
    int t9 = t8 * t5 - t3;
    int t10 = (t9 << 1) | 0x1;
    
    /* More computations */
    int t11 = t10 + t4;
    int t12 = t11 * 3;
    asm volatile("" : : "r"(t12));
    
    int t13 = t12 / 2 + t7;
    int t14 = t13 ^ t8;
    
    /* Another function call */
    int t15 = ext_func1(t14);
    
    int t16 = t15 * 2 - t11;
    result = t16 + t9;
    
    return result;
}

/* Test 2: Floating point and mixed mode operations */
double test_floating_pressure(double seed) {
    volatile double result = 0.0;
    double a = seed * 1.5;
    double b = seed / 3.0;
    
    double f1 = a * b + seed;
    asm volatile("" : : "r"(f1));
    
    double f2 = f1 / 2.0 - b;
    double f3 = f2 * 3.14159;
    
    /* Function call with floating point */
    double f4 = ext_func2(f3);
    
    double f5 = f4 + a * 2.0;
    asm volatile("" : : "r"(f5));
    
    double f6 = f5 - f2;
    double f7 = f6 * f3;
    
    /* Mix with integer */
    int i1 = (int)f7;
    double f8 = f7 + i1;
    
    double f9 = ext_func2(f8);
    double f10 = f9 * 2.0 - f5;
    
    result = f10 + f6;
    return result;
}

/* Test 3: Nested loops with complex induction */
long long test_nested_loops(int N) {
    volatile long long sum = 0;
    
    for (int i = 0; i < N; i++) {
        /* Complex induction variable */
        for (int j = i * 2; j < N; j += 3) {
            /* Inner computations increase pressure */
            int t1 = i * j + 5;
            int t2 = t1 & 0xFF;
            asm volatile("" : : "r"(t2));
            
            int t3 = t2 * 3 - j;
            int t4 = (t3 << 2) | 1;
            
            /* Function call in inner loop */
            long long t5 = ext_func3(t4);
            
            int t6 = t4 + i;
            long long t7 = t5 * t6;
            asm volatile("" : : "r"(t7));
            
            int t8 = t6 ^ j;
            long long t9 = t7 - t8;
            
            sum += t9;
        }
        
        /* Additional computation between loops */
        int outer_temp = i * 7 + 3;
        asm volatile("" : : "r"(outer_temp));
        sum += outer_temp;
    }
    
    return sum;
}

/* Test 4: 64-bit and vector operations */
long long test_64bit_vector(int seed) {
    volatile long long result = 0;
    
    /* 64-bit operations */
    long long ll1 = (long long)seed * 1000000007LL;
    long long ll2 = ll1 >> 16;
    asm volatile("" : : "r"(ll2));
    
    long long ll3 = ll2 + 123456789LL;
    long long ll4 = ll3 * 3;
    
    /* Function call with 64-bit */
    long long ll5 = ext_func3(ll4);
    
    long long ll6 = ll5 - ll2;
    asm volatile("" : : "r"(ll6));
    
    /* Vector operations */
    v4si v1 = {seed, seed + 1, seed + 2, seed + 3};
    v4si v2 = {5, 6, 7, 8};
    v4si v3 = v1 * v2;
    
    /* Extract and use vector elements */
    int ve1 = v3[0] + v3[1];
    int ve2 = v3[2] * v3[3];
    asm volatile("" : : "r"(ve1), "r"(ve2));
    
    long long ll7 = (long long)ve1 * ve2;
    result = ll6 + ll7;
    
    return result;
}

/* Test 5: Extreme register pressure with many live values */
int test_extreme_pressure(int seed) {
    volatile int result = 0;
    
    /* Create many independent live values */
    int v1 = seed * 2;
    int v2 = seed + 17;
    int v3 = v1 & v2;
    int v4 = v3 | 0x55;
    int v5 = v4 << 3;
    int v6 = v5 - v2;
    int v7 = v6 * 7;
    int v8 = v7 ^ v1;
    int v9 = v8 >> 1;
    int v10 = v9 + v4;
    
    /* Pin all values to increase pressure */
    asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4), "r"(v5));
    asm volatile("" : : "r"(v6), "r"(v7), "r"(v8), "r"(v9), "r"(v10));
    
    /* Function call with many live values */
    int r1 = ext_func1(v10);
    
    /* More computations */
    int v11 = r1 * v1;
    int v12 = v11 + v2;
    int v13 = v12 & v3;
    int v14 = v13 | v4;
    int v15 = v14 << 2;
    
    asm volatile("" : : "r"(v11), "r"(v12), "r"(v13), "r"(v14), "r"(v15));
    
    int r2 = ext_func1(v15);
    
    result = r2 + v10;
    return result;
}

/* Dummy external functions */
int ext_func1(int x) { return x * 2 + 1; }
double ext_func2(double x) { return x * 1.5 - 2.0; }
long long ext_func3(long long x) { return x / 3 + 100; }

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    int N = argc > 2 ? atoi(argv[2]) : 100;
    
    printf("Testing early rematerialization coverage...\n");
    
    /* Run all tests to ensure code execution */
    int r1 = test_mixed_integer_pressure(seed);
    printf("Test 1 result: %d\n", r1);
    
    double r2 = test_floating_pressure(seed * 1.0);
    printf("Test 2 result: %f\n", r2);
    
    long long r3 = test_nested_loops(N);
    printf("Test 3 result: %lld\n", r3);
    
    long long r4 = test_64bit_vector(seed);
    printf("Test 4 result: %lld\n", r4);
    
    int r5 = test_extreme_pressure(seed);
    printf("Test 5 result: %d\n", r5);
    
    /* Final checksum to prevent optimization */
    volatile int checksum = r1 + (int)r2 + (int)r3 + (int)r4 + r5;
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
