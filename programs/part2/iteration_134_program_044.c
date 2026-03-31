/* test_early_remat.c - Test program for GCC early rematerialization pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External non-inline functions to force register pressure */
extern int ext_func1(int);
extern double ext_func2(double);
extern long long ext_func3(long long);

/* Vector type to test different modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High integer register pressure with complex expressions */
int test_high_int_pressure(int seed) {
    volatile int barrier = seed; /* Prevent optimization */
    int a = barrier + 1;
    int b = barrier * 2;
    int c = barrier / 3;
    int d = barrier - 4;
    
    /* Long sequence of independent computations */
    int t1 = a * b + c;
    asm volatile("" : : "r"(t1)); /* Pin in register */
    
    int t2 = (b & c) | (d ^ a);
    asm volatile("" : : "r"(t2));
    
    int t3 = t1 * t2 - a;
    int t4 = t2 / (b + 1) + c;
    int t5 = (t3 << 2) | (t4 >> 1);
    
    /* Function call clobbers caller-saved registers */
    int t6 = ext_func1(t5);
    
    int t7 = t6 * a + b;
    int t8 = t7 & 0xFF;
    int t9 = t8 * t4 - t3;
    int t10 = (t9 + t6) * 2;
    
    asm volatile("" : : "r"(t7), "r"(t8), "r"(t9));
    
    int t11 = ext_func1(t10);
    int t12 = t11 * 3 + t10;
    int t13 = (t12 << 3) ^ t9;
    int t14 = t13 / (t8 + 1);
    int t15 = t14 | t7;
    
    /* More computations to increase pressure */
    for (int i = 0; i < 4; i++) {
        t15 = t15 * (t14 - i) + t13;
        asm volatile("" : : "r"(t15));
    }
    
    return t15 + t10 + t5;
}

/* Test 2: Mixed floating-point and integer operations */
double test_mixed_types(int seed) {
    volatile double dbarrier = seed * 1.5;
    double da = dbarrier + 1.1;
    double db = dbarrier * 2.2;
    float fc = dbarrier / 3.3f;
    float fd = dbarrier - 4.4f;
    
    double dt1 = da * db + fc;
    asm volatile("" : : "r"(dt1)); /* Pin in FP register */
    
    float ft2 = (float)(db * 0.5) + fd;
    int it3 = (int)(dt1 * 100.0);
    
    /* Function call with double */
    double dt4 = ext_func2(dt1);
    
    double dt5 = dt4 * da + db;
    float ft6 = (float)dt5 * fc;
    int it7 = (int)ft6 * it3;
    
    asm volatile("" : : "r"(dt5), "r"(ft6), "r"(it7));
    
    /* More mixed computations */
    for (int i = 0; i < 3; i++) {
        dt5 = dt5 * (1.0 + i * 0.1) + dt4;
        ft6 = ft6 * (fc + i * 0.5f);
        it7 = it7 + (int)(dt5 * ft6);
        asm volatile("" : : "r"(dt5), "r"(ft6));
    }
    
    return dt5 + ft6 + it7;
}

/* Test 3: Nested loops with complex induction variables */
long long test_nested_loops(int N) {
    long long total = 0;
    
    for (int i = 0; i < N; i++) {
        /* Complex induction variable */
        for (int j = i * 2; j < N; j += 3) {
            int a = i * j + 1;
            int b = (j << 1) ^ i;
            int c = a * b - j;
            
            asm volatile("" : : "r"(a), "r"(b), "r"(c));
            
            /* Function call inside inner loop */
            long long d = ext_func3(c);
            
            for (int k = j; k > 0; k -= 2) {
                int e = k * a + b;
                int f = e & 0xFF;
                long long g = d * f + k;
                
                asm volatile("" : : "r"(e), "r"(f), "r"(g));
                
                total += g + e + f;
                
                /* More computations to increase pressure */
                int h = (e << 2) | (f >> 1);
                long long m = g * h - total;
                asm volatile("" : : "r"(h), "r"(m));
                
                total += m % 1000;
            }
            
            total += d;
        }
        
        /* Additional computation between outer loop iterations */
        int x = i * 7 + 3;
        int y = (x ^ 0x55) * 11;
        asm volatile("" : : "r"(x), "r"(y));
        
        total += x * y;
    }
    
    return total;
}

/* Test 4: Vector operations for vector modes */
v4si test_vector_ops(int seed) {
    v4si v1 = {seed, seed + 1, seed + 2, seed + 3};
    v4si v2 = {seed * 2, seed * 3, seed * 4, seed * 5};
    v4si v3 = {seed + 10, seed + 20, seed + 30, seed + 40};
    
    /* Vector operations */
    v4si r1 = v1 + v2;
    v4si r2 = v1 * v3;
    v4si r3 = r1 & v2;
    v4si r4 = r2 | r3;
    
    /* Use inline asm to pin vector values */
    asm volatile("" : : "r"(r1), "r"(r2), "r"(r3));
    
    /* Scalar computations mixed with vector */
    int s1 = seed * 100;
    int s2 = s1 + (int)r1[0];
    int s3 = s2 * (int)r2[1];
    
    asm volatile("" : : "r"(s1), "r"(s2), "r"(s3));
    
    /* More vector operations */
    for (int i = 0; i < 4; i++) {
        r4[i] = r4[i] * s3 + v1[i];
        v1[i] = v1[i] ^ (s2 + i);
    }
    
    /* Function call to increase pressure */
    int t = ext_func1(s3);
    
    v4si result = r4 + v1;
    for (int i = 0; i < 4; i++) {
        result[i] += t * i;
    }
    
    return result;
}

/* Test 5: Extreme register pressure with all types */
long long test_extreme_pressure(int seed) {
    /* Use all types to engage different modes */
    char c1 = seed & 0xFF;
    short s1 = seed * 2;
    int i1 = seed * 3;
    long long ll1 = seed * 100LL;
    float f1 = seed * 0.5f;
    double d1 = seed * 1.5;
    
    /* Many independent computations */
    char c2 = c1 + 1;
    short s2 = s1 * c2;
    int i2 = i1 + s2;
    long long ll2 = ll1 * i2;
    float f2 = f1 * i2;
    double d2 = d1 + f2;
    
    asm volatile("" : : "r"(c2), "r"(s2), "r"(i2), "r"(ll2));
    asm volatile("" : : "r"(f2), "r"(d2));
    
    /* Chain of dependent computations */
    for (int i = 0; i < 8; i++) {
        c2 = c2 * 3 + i;
        s2 = s2 + c2;
        i2 = i2 ^ s2;
        ll2 = ll2 + i2 * i;
        f2 = f2 * (1.0f + i * 0.1f);
        d2 = d2 + f2 / (i + 1);
        
        asm volatile("" : : "r"(c2), "r"(s2), "r"(i2));
        asm volatile("" : : "r"(ll2), "r"(f2), "r"(d2));
        
        /* Function calls at pressure points */
        if (i % 3 == 0) {
            i2 = ext_func1(i2);
            d2 = ext_func2(d2);
        }
    }
    
    /* Final computation mixing all types */
    long long result = ll2 + (long long)(d2 * 1000.0) + 
                      (long long)(f2 * 100.0f) + i2 + s2 + c2;
    
    return result;
}

/* Dummy external functions */
int ext_func1(int x) { return x * 3 + 1; }
double ext_func2(double x) { return x * 2.5 - 1.0; }
long long ext_func3(long long x) { return x * 5LL + 2LL; }

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : time(NULL);
    long long total = 0;
    
    printf("Testing early rematerialization patterns...\n");
    
    /* Run all tests to maximize coverage chances */
    total += test_high_int_pressure(seed);
    printf("Test 1 complete: %lld\n", total);
    
    total += (long long)test_mixed_types(seed);
    printf("Test 2 complete: %lld\n", total);
    
    total += test_nested_loops(seed % 10 + 5);
    printf("Test 3 complete: %lld\n", total);
    
    v4si vec_result = test_vector_ops(seed);
    total += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
    printf("Test 4 complete: %lld\n", total);
    
    total += test_extreme_pressure(seed);
    printf("Test 5 complete: %lld\n", total);
    
    /* Use result to prevent optimization */
    volatile long long final_result = total;
    printf("Final checksum: %lld\n", final_result);
    
    return (final_result > 0) ? 0 : 1;
}
