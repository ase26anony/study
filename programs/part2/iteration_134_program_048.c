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
double ext_func3(double x) { return x * 3.1415926535; }
long long ext_func4(long long x) { return x + 0xFFFFFFFF; }

/* Vector type to test different modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High integer register pressure with complex expressions */
int test_integer_pressure(int seed) {
    volatile int result = 0;
    int a = seed * 3 + 1;
    int b = seed / 2 - 5;
    int c = seed ^ 0xABCD;
    int d = seed + 777;
    
    /* Long sequence of independent computations */
    int t1 = a * b + c - d;
    asm volatile("" : : "r"(t1)); /* Force t1 into register */
    
    int t2 = (a & b) | (c ^ d);
    int t3 = t1 * t2 - seed;
    asm volatile("" : : "r"(t3));
    
    /* Function call creates register pressure point */
    int t4 = ext_func1(t3);
    
    int t5 = (t2 << 3) | (t4 >> 2);
    int t6 = t5 * 7 + 11;
    asm volatile("" : : "r"(t6));
    
    int t7 = ext_func2(t6, t4);
    
    int t8 = t7 * 13 - t5;
    int t9 = (t8 ^ t6) & 0xFF;
    asm volatile("" : : "r"(t9));
    
    int t10 = t9 * 19 + 23;
    int t11 = (t10 % 31) * t8;
    asm volatile("" : : "r"(t11));
    
    /* More computations with different patterns */
    for (int i = 0; i < 8; i++) {
        t11 = t11 * 2 + i;
        asm volatile("" : : "r"(t11));
        t10 = ext_func1(t10 + i);
    }
    
    result = t11 ^ t10 ^ t9 ^ t8;
    return result;
}

/* Test 2: Mixed integer and floating-point pressure */
double test_mixed_pressure(int seed) {
    volatile double result = 0.0;
    double f1 = seed * 1.5;
    double f2 = seed / 3.14159;
    int i1 = seed * 2;
    int i2 = seed + 1000;
    
    /* Mix FP and integer operations */
    double ft1 = f1 * f2 + 1.2345;
    asm volatile("" : : "r"(ft1));
    
    int it1 = i1 * i2 - 54321;
    asm volatile("" : : "r"(it1));
    
    double ft2 = ext_func3(ft1);
    int it2 = ext_func1(it1);
    
    double ft3 = ft2 * (double)it2 / 2.0;
    asm volatile("" : : "r"(ft3));
    
    int it3 = (int)ft3 * it1;
    asm volatile("" : : "r"(it3));
    
    /* Nested loops with mixed types */
    for (int i = 0; i < 4; i++) {
        for (int j = i * 2; j < 8; j += 3) { /* Complex induction */
            double temp = ft3 * j + i;
            asm volatile("" : : "r"(temp));
            ft3 = ext_func3(temp);
            it3 = ext_func2(it3, j);
        }
        asm volatile("" : : "r"(ft3), "r"(it3));
    }
    
    result = ft3 + (double)it3;
    return result;
}

/* Test 3: Long long and 64-bit operations */
long long test_64bit_pressure(int seed) {
    volatile long long result = 0;
    long long ll1 = (long long)seed * 1000000007LL;
    long long ll2 = (long long)seed * 998244353LL;
    
    /* Chain of 64-bit computations */
    long long t1 = ll1 * ll2 + 1234567890123LL;
    asm volatile("" : : "r"(t1));
    
    long long t2 = (ll1 ^ ll2) | 0x5555555555555555LL;
    asm volatile("" : : "r"(t2));
    
    long long t3 = ext_func4(t1);
    
    long long t4 = t2 * 7LL - t3;
    asm volatile("" : : "r"(t4));
    
    long long t5 = (t4 << 5) | (t3 >> 3);
    long long t6 = t5 * 11LL + 13LL;
    asm volatile("" : : "r"(t6));
    
    /* Multiple function calls */
    for (int i = 0; i < 6; i++) {
        t6 = ext_func4(t6 + i);
        asm volatile("" : : "r"(t6));
        t5 = t5 * 3LL + t6;
    }
    
    result = t5 ^ t6;
    return result;
}

/* Test 4: Vector operations for vector modes */
int test_vector_pressure(int seed) {
    volatile int result = 0;
    v4si v1 = {seed, seed + 1, seed + 2, seed + 3};
    v4si v2 = {seed * 2, seed * 3, seed * 4, seed * 5};
    
    /* Vector operations */
    v4si vt1 = v1 + v2;
    v4si vt2 = v1 * v2;
    asm volatile("" : : "r"(vt1), "r"(vt2));
    
    /* Scalar operations mixed with vector */
    int s1 = seed * 7;
    int s2 = seed + 11;
    
    for (int i = 0; i < 4; i++) {
        v4si temp = vt1 * i + vt2;
        asm volatile("" : : "r"(temp));
        vt1 = temp + v1;
        s1 = ext_func1(s1 + i);
        s2 = ext_func2(s2, i);
    }
    
    /* Extract and combine results */
    int arr[4];
    __builtin_memcpy(arr, &vt1, sizeof(vt1));
    result = arr[0] + arr[1] + arr[2] + arr[3] + s1 + s2;
    
    return result;
}

/* Test 5: Extreme register pressure with many live values */
int test_extreme_pressure(int seed) {
    volatile int result = 0;
    
    /* Create many independent variables */
    int v[20];
    for (int i = 0; i < 20; i++) {
        v[i] = seed * (i + 1) + i * i;
    }
    
    /* Long computation chain with all variables live */
    int t = v[0];
    for (int i = 1; i < 20; i++) {
        t = t * 3 + v[i];
        asm volatile("" : : "r"(t)); /* Force each intermediate to register */
        
        if (i % 3 == 0) {
            t = ext_func1(t); /* Function call adds pressure */
        }
        
        if (i % 5 == 0) {
            t = ext_func2(t, v[i-1]);
        }
    }
    
    /* Nested loops with complex induction */
    for (int i = 0; i < 10; i++) {
        for (int j = i * 2; j < i * 2 + 5; j += 1) {
            for (int k = j; k < j + 3; k++) {
                t = t + v[k % 20] * (i + j + k);
                asm volatile("" : : "r"(t));
            }
            t = ext_func1(t);
        }
    }
    
    result = t;
    return result;
}

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : time(NULL);
    int total = 0;
    
    printf("Testing early rematerialization coverage...\n");
    
    /* Run all tests to exercise different patterns */
    total += test_integer_pressure(seed);
    printf("Integer test result: %d\n", test_integer_pressure(seed));
    
    double dres = test_mixed_pressure(seed);
    total += (int)dres;
    printf("Mixed test result: %f\n", dres);
    
    long long llres = test_64bit_pressure(seed);
    total += (int)llres;
    printf("64-bit test result: %lld\n", llres);
    
    total += test_vector_pressure(seed);
    printf("Vector test result: %d\n", test_vector_pressure(seed));
    
    total += test_extreme_pressure(seed);
    printf("Extreme test result: %d\n", test_extreme_pressure(seed));
    
    /* Use result to prevent optimization */
    volatile int final_result = total;
    printf("Final checksum: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
