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
    int c = seed ^ 0x1234;
    int d = seed + 777;
    
    /* Long sequence of independent computations */
    int t1 = a * b + c - d;
    asm volatile("" : : "r"(t1)); /* Force into register */
    
    int t2 = (b << 3) | (c >> 2);
    int t3 = t1 ^ t2;
    asm volatile("" : : "r"(t3));
    
    /* Function call creates pressure point */
    int t4 = ext_func1(t3);
    
    int t5 = t4 * 7 + 12345;
    int t6 = t5 & 0xFFFF;
    asm volatile("" : : "r"(t6));
    
    int t7 = t6 - a + b;
    int t8 = t7 * t7;
    asm volatile("" : : "r"(t8));
    
    int t9 = ext_func1(t8);
    
    int t10 = t9 | 0xFF00;
    int t11 = t10 * 3;
    int t12 = t11 / 2;
    asm volatile("" : : "r"(t12));
    
    int t13 = t12 + seed;
    int t14 = t13 ^ 0xAAAAAAAA;
    asm volatile("" : : "r"(t14));
    
    int t15 = ext_func1(t14);
    
    /* Nested loops with complex induction */
    for (int i = 0; i < 100; i++) {
        for (int j = i * 2; j < 100; j += 3) {
            int k = j * 3 - i;
            t15 += k;
            asm volatile("" : : "r"(k));
        }
        /* Another function call in loop */
        t15 = ext_func1(t15) + i;
    }
    
    result = t15;
    return result;
}

/* Test 2: Mixed floating-point and integer pressure */
double test_mixed_pressure(double seed) {
    volatile double result = 0.0;
    double a = seed * 1.5;
    double b = seed / 3.0;
    int c = (int)seed * 2;
    
    /* Mix float and int operations */
    double t1 = a * b + 1.234;
    asm volatile("" : : "r"(t1));
    
    int t2 = c * 3 + 456;
    double t3 = t1 + (double)t2;
    asm volatile("" : : "r"(t3));
    
    /* Function call with double */
    double t4 = ext_func2(t3);
    
    float t5 = (float)t4 * 2.5f;
    double t6 = t4 + (double)t5;
    asm volatile("" : : "r"(t6));
    
    int t7 = (int)t6 * 7;
    double t8 = t6 * (double)t7;
    asm volatile("" : : "r"(t8));
    
    /* Another external call */
    double t9 = ext_func2(t8);
    
    /* Nested loops with mixed types */
    for (int i = 0; i < 50; i++) {
        for (double j = i * 1.5; j < 100.0; j += 2.5) {
            double k = j * 3.14 - (double)i;
            t9 += k;
            asm volatile("" : : "r"(k));
        }
        t9 = ext_func2(t9) + (double)i;
    }
    
    result = t9;
    return result;
}

/* Test 3: Long long and 64-bit operations */
long long test_64bit_pressure(long long seed) {
    volatile long long result = 0;
    long long a = seed * 3LL;
    long long b = seed / 2LL;
    long long c = seed ^ 0x123456789ABCDEFLL;
    
    /* 64-bit computations */
    long long t1 = a * b + c;
    asm volatile("" : : "r"(t1));
    
    long long t2 = (b << 5) | (c >> 3);
    long long t3 = t1 ^ t2;
    asm volatile("" : : "r"(t3));
    
    /* External call with long long */
    long long t4 = ext_func3(t3);
    
    long long t5 = t4 * 13LL + 123456789LL;
    long long t6 = t5 & 0xFFFFFFFFLL;
    asm volatile("" : : "r"(t6));
    
    long long t7 = t6 - a + b;
    long long t8 = t7 * t7;
    asm volatile("" : : "r"(t8));
    
    long long t9 = ext_func3(t8);
    
    /* Complex nested loops */
    for (long long i = 0; i < 30; i++) {
        for (long long j = i * 3; j < 50; j += 4) {
            long long k = j * 5 - i * 2;
            t9 += k;
            asm volatile("" : : "r"(k));
        }
        t9 = ext_func3(t9) + i;
    }
    
    result = t9;
    return result;
}

/* Test 4: Vector operations for vector modes */
v4si test_vector_pressure(v4si seed) {
    v4si a = seed + (v4si){1, 2, 3, 4};
    v4si b = seed * (v4si){2, 3, 4, 5};
    
    /* Vector computations */
    v4si t1 = a * b + seed;
    asm volatile("" : : "r"(t1));
    
    v4si t2 = t1 << (v4si){1, 2, 1, 2};
    v4si t3 = t2 | (v4si){0xFF, 0xFF00, 0xFF0000, 0xFF000000};
    asm volatile("" : : "r"(t3));
    
    /* Mix with scalar operations */
    int scalar = t3[0] + t3[1];
    v4si t4 = t3 + (v4si){scalar, scalar, scalar, scalar};
    asm volatile("" : : "r"(t4));
    
    /* Nested loops with vector operations */
    for (int i = 0; i < 20; i++) {
        v4si loop_var = {i, i*2, i*3, i*4};
        t4 = t4 + loop_var * (v4si){1, -1, 1, -1};
        asm volatile("" : : "r"(loop_var));
    }
    
    return t4;
}

/* Test 5: Extreme register pressure with all types */
long long test_extreme_pressure(int seed1, double seed2, long long seed3) {
    volatile long long final_result = 0;
    
    /* Integer computations */
    int a = seed1 * 5 + 1;
    int b = seed1 / 3 - 2;
    int c = a ^ b;
    asm volatile("" : : "r"(c));
    
    /* Floating computations */
    double x = seed2 * 2.71828;
    double y = seed2 / 3.14159;
    double z = x * y;
    asm volatile("" : : "r"(z));
    
    /* 64-bit computations */
    long long m = seed3 * 7LL;
    long long n = seed3 / 5LL;
    long long p = m ^ n;
    asm volatile("" : : "r"(p));
    
    /* Function calls between dependent computations */
    int c2 = ext_func1(c);
    double z2 = ext_func2(z);
    long long p2 = ext_func3(p);
    
    /* More computations mixing types */
    int d = c2 * 11 + (int)z2;
    double w = z2 * 3.0 + (double)p2;
    long long q = p2 + (long long)d;
    
    asm volatile("" : : "r"(d), "r"(w), "r"(q));
    
    /* Triple nested loops for maximum pressure */
    for (int i = 0; i < 10; i++) {
        for (int j = i * 2; j < 15; j += 2) {
            for (int k = j * 3; k < 20; k += 3) {
                int tmp1 = i * j * k + d;
                double tmp2 = (double)tmp1 * w;
                long long tmp3 = (long long)tmp2 + q;
                
                d += tmp1;
                w += tmp2;
                q += tmp3;
                
                asm volatile("" : : "r"(tmp1), "r"(tmp2), "r"(tmp3));
            }
            /* Function call in middle loop */
            d = ext_func1(d);
        }
        /* More function calls */
        w = ext_func2(w);
        q = ext_func3(q);
    }
    
    final_result = (long long)d + (long long)w + q;
    return final_result;
}

int main(int argc, char *argv[]) {
    int seed_int = argc > 1 ? atoi(argv[1]) : 12345;
    double seed_double = argc > 2 ? atof(argv[2]) : 3.14159;
    long long seed_ll = argc > 3 ? atoll(argv[3]) : 9876543210LL;
    
    printf("Testing early rematerialization coverage...\n");
    
    /* Run all tests to create various pressure scenarios */
    int res1 = test_integer_pressure(seed_int);
    printf("Test 1 result: %d\n", res1);
    
    double res2 = test_mixed_pressure(seed_double);
    printf("Test 2 result: %f\n", res2);
    
    long long res3 = test_64bit_pressure(seed_ll);
    printf("Test 3 result: %lld\n", res3);
    
    v4si seed_vec = {seed_int, seed_int+1, seed_int+2, seed_int+3};
    v4si res4 = test_vector_pressure(seed_vec);
    printf("Test 4 result: {%d, %d, %d, %d}\n", res4[0], res4[1], res4[2], res4[3]);
    
    long long res5 = test_extreme_pressure(seed_int, seed_double, seed_ll);
    printf("Test 5 result: %lld\n", res5);
    
    /* Use results to prevent optimization */
    volatile int final_check = res1 + (int)res2 + (int)res3 + res4[0] + (int)res5;
    printf("Final check: %d\n", final_check);
    
    return 0;
}
