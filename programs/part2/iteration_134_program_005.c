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
int ext_func2(int x, int y) { return x * y + (x ^ y); }
double ext_func3(double x) { return x * 1.234567; }
long long ext_func4(long long x) { return x * 3 + 1; }

/* Vector type to test different modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High integer register pressure with complex expressions */
int test_high_int_pressure(int seed) {
    volatile int result = 0;
    int a = seed + 1;
    int b = seed * 2;
    int c = seed / 3;
    int d = seed - 100;
    int e = seed ^ 0x12345678;
    int f = seed | 0x87654321;
    int g = seed & 0xF0F0F0F0;
    int h = seed << 3;
    int i = seed >> 2;
    int j = ~seed;
    
    /* Force all values into registers */
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e));
    asm volatile("" : : "r"(f), "r"(g), "r"(h), "r"(i), "r"(j));
    
    /* Long sequence of independent computations */
    int t1 = a * b + c - d;
    int t2 = e & f | g ^ h;
    int t3 = i << (j & 0xF);
    int t4 = t1 * t2 - t3;
    int t5 = (t1 ^ t2) & (t3 | t4);
    int t6 = t4 * 7 + t5 / 3;
    int t7 = t5 << 2 | t6 >> 1;
    int t8 = t6 * t7 - t4 * t5;
    int t9 = (t7 ^ t8) + (t6 & t7);
    int t10 = t8 * 13 - t9 * 11;
    
    /* Function call that clobbers caller-saved registers */
    int r1 = ext_func1(t1);
    asm volatile("" : : "r"(t2), "r"(t3), "r"(t4), "r"(t5));
    
    int t11 = t10 + r1 * 2;
    int t12 = t9 * 3 + t11 / 5;
    int t13 = (t10 ^ t11) & (t12 | t8);
    int t14 = t12 << 3 | t13 >> 2;
    
    /* Another function call */
    int r2 = ext_func2(t6, t7);
    asm volatile("" : : "r"(t8), "r"(t9), "r"(t10), "r"(t11));
    
    int t15 = t14 * r2 - t13 * 7;
    int t16 = (t14 ^ t15) + (t12 & t13);
    int t17 = t15 * 17 + t16 / 19;
    int t18 = t16 << 4 | t17 >> 3;
    
    result = t17 + t18 + r1 + r2;
    asm volatile("" : "+r"(result));
    
    return result;
}

/* Test 2: Mixed integer and floating-point operations */
double test_mixed_types(int seed) {
    volatile double result = 0.0;
    int i1 = seed * 2;
    int i2 = seed + 100;
    int i3 = seed ^ 0xABCD;
    double f1 = seed * 1.5;
    double f2 = seed / 2.0;
    double f3 = seed + 3.14159;
    
    asm volatile("" : : "r"(i1), "r"(i2), "r"(i3));
    asm volatile("" : : "f"(f1), "f"(f2), "f"(f3));
    
    /* Mixed computations */
    double t1 = f1 * f2 + f3;
    int t2 = i1 * i2 - i3;
    double t3 = t1 * i1;
    int t4 = t2 + (int)f2;
    double t5 = f3 * t4;
    int t6 = i3 ^ (int)t1;
    
    /* Function call with floating point */
    double r1 = ext_func3(t1);
    asm volatile("" : : "f"(t3), "f"(t5), "r"(t2), "r"(t4));
    
    double t7 = t3 + r1 * 2.0;
    int t8 = t6 * 3 + (int)t5;
    double t9 = t7 * f1 - f2;
    int t10 = t8 ^ (int)t7;
    double t11 = t9 / 1.234 + t5;
    
    /* Another call */
    double r2 = ext_func3(t7);
    asm volatile("" : : "f"(t9), "f"(t11), "r"(t8), "r"(t10));
    
    double t12 = t11 * r2 + t9;
    int t13 = t10 * 5 - (int)t11;
    double t14 = t12 * 3.14159;
    
    result = t12 + t14 + r1 + r2 + t13;
    asm volatile("" : "+f"(result));
    
    return result;
}

/* Test 3: Nested loops with complex induction variables */
long long test_nested_loops(int N) {
    volatile long long total = 0;
    
    for (int i = 0; i < N; i++) {
        /* Complex induction variable */
        for (int j = i * 2; j < N; j += 3) {
            /* Inner loop with register pressure */
            int a = i * j + 1;
            int b = (i ^ j) * 3;
            int c = a * b - j;
            int d = (a & b) | (c ^ i);
            
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            
            /* More computations in inner loop */
            int e = d * 7 + a / 3;
            int f = (b << 2) | (c >> 1);
            int g = e * f - d * 5;
            int h = (e ^ f) + (g & d);
            
            /* Function call inside inner loop */
            int r = ext_func1(g);
            asm volatile("" : : "r"(e), "r"(f), "r"(g), "r"(h));
            
            int k = h * r + e * 2;
            int l = (f ^ h) & (g | k);
            int m = k * 11 - l * 7;
            
            total += m + r + j;
            
            /* Vector operations for different modes */
            if ((j % 8) == 0) {
                v4si v1 = {a, b, c, d};
                v4si v2 = {e, f, g, h};
                v4si v3 = v1 + v2;
                v4si v4 = v1 * v2;
                
                /* Use vector results */
                for (int n = 0; n < 4; n++) {
                    total += v3[n] + v4[n];
                }
            }
        }
        
        /* External call in outer loop */
        if (i % 5 == 0) {
            long long r = ext_func4(i);
            total += r * 2;
        }
    }
    
    asm volatile("" : "+r"(total));
    return total;
}

/* Test 4: Long dependency chain with register pressure */
long long test_dependency_chain(int seed) {
    volatile long long acc = seed;
    
    /* Create long dependency chain */
    long long v0 = seed;
    asm volatile("" : "+r"(v0));
    
    for (int i = 0; i < 50; i++) {
        /* Each iteration depends on previous */
        long long v1 = v0 * 6364136223846793005ULL + 1442695040888963407ULL;
        long long v2 = v1 ^ (v1 >> 21);
        long long v3 = v2 ^ (v2 << 35);
        long long v4 = v3 ^ (v3 >> 4);
        long long v5 = v4 * 2685821657736338717ULL;
        
        /* Additional parallel computations */
        int t1 = (v1 & 0xFFFFFFFF) * (i + 1);
        int t2 = (v2 >> 32) ^ i;
        double f1 = (double)v3 * 0.0000001;
        double f2 = (double)v4 * 1.234567e-7;
        
        asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4), "r"(v5));
        asm volatile("" : : "r"(t1), "r"(t2), "f"(f1), "f"(f2));
        
        /* Function calls at intervals */
        if (i % 7 == 0) {
            int r = ext_func1(t1);
            t2 += r;
        }
        
        if (i % 11 == 0) {
            double r = ext_func3(f1);
            f2 += r;
        }
        
        /* Mix results back */
        v0 = v5 + t1 + t2 + (long long)f1 + (long long)f2;
        acc += v0;
    }
    
    asm volatile("" : "+r"(acc));
    return acc;
}

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    int N = argc > 2 ? atoi(argv[2]) : 100;
    
    printf("Testing early rematerialization coverage...\n");
    
    /* Run all tests to increase coverage chances */
    int r1 = test_high_int_pressure(seed);
    printf("Test 1 result: %d\n", r1);
    
    double r2 = test_mixed_types(seed);
    printf("Test 2 result: %f\n", r2);
    
    long long r3 = test_nested_loops(N);
    printf("Test 3 result: %lld\n", r3);
    
    long long r4 = test_dependency_chain(seed);
    printf("Test 4 result: %lld\n", r4);
    
    /* Final volatile store to prevent optimization */
    volatile int final = r1 + (int)r2 + (int)r3 + (int)r4;
    asm volatile("" : : "r"(final));
    
    printf("All tests completed.\n");
    return 0;
}
