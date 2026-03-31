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

/* Test function 1: High integer register pressure with complex expressions */
int test_high_int_pressure(int seed) {
    volatile int result = 0;
    int a = seed * 3 + 1;
    int b = seed / 2 - 5;
    int c = seed + 12345;
    int d = seed ^ 0xABCDEF;
    int e = seed | 0x123456;
    int f = seed & 0xF0F0F0F0;
    
    /* Force values into registers */
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f));
    
    /* Long sequence of independent computations */
    int t1 = a * b + c - d;
    int t2 = (e & f) | (c ^ d);
    int t3 = t1 * t2 / (a + 1);
    int t4 = (b << 3) | (c >> 2);
    int t5 = t3 ^ t4 + d;
    int t6 = e * f - t1 + t2;
    int t7 = (t5 & 0xFF) | (t6 << 8);
    int t8 = t3 * t4 * t5 * t6;
    int t9 = t7 + t8 - a * b;
    int t10 = (t9 << 1) | (t9 >> 31);
    
    asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4), "r"(t5));
    asm volatile("" : : "r"(t6), "r"(t7), "r"(t8), "r"(t9), "r"(t10));
    
    /* Function call that clobbers registers */
    int r1 = ext_func1(t1);
    
    /* More computations after call */
    int t11 = t10 * r1 + t9;
    int t12 = (t11 ^ t8) & t7;
    int t13 = t6 * t5 - t4 + t3;
    int t14 = t2 << t1;
    int t15 = t13 | t14 ^ t12;
    
    asm volatile("" : : "r"(t11), "r"(t12), "r"(t13), "r"(t14), "r"(t15));
    
    /* Another function call */
    int r2 = ext_func1(t11);
    
    /* Final computation */
    result = t15 + r1 + r2;
    asm volatile("" : "+r"(result));
    
    return result;
}

/* Test function 2: Mixed integer and floating-point pressure */
double test_mixed_pressure(double seed) {
    volatile double result = 0.0;
    double d1 = seed * 1.234;
    double d2 = seed / 0.987;
    double d3 = seed + 456.789;
    float f1 = (float)seed * 2.5f;
    float f2 = (float)seed / 3.14f;
    
    asm volatile("" : : "r"(d1), "r"(d2), "r"(d3), "r"(f1), "r"(f2));
    
    /* Mixed computations */
    double t1 = d1 * d2 + d3;
    float t2 = f1 - f2 * 2.0f;
    double t3 = t1 / d2 - d3;
    float t4 = t2 + f1 / f2;
    double t5 = t3 * 3.14159;
    float t6 = t4 * 0.5f;
    
    asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4), "r"(t5), "r"(t6));
    
    /* External function call with floating point */
    double r1 = ext_func2(t1);
    
    /* More mixed computations */
    double t7 = t5 + r1 * t3;
    float t8 = t6 - (float)r1 + t4;
    double t9 = t7 / 2.0 + t5;
    float t10 = t8 * 3.0f - t6;
    
    asm volatile("" : : "r"(t7), "r"(t8), "r"(t9), "r"(t10));
    
    result = t9 + (double)t10;
    asm volatile("" : "+r"(result));
    
    return result;
}

/* Test function 3: Nested loops with complex induction */
long long test_nested_loops(int N) {
    volatile long long sum = 0;
    
    for (int i = 0; i < N; i++) {
        /* Complex induction variable */
        for (int j = i * 2; j < N; j += 3) {
            /* Inner loop with high pressure */
            int a = i * j + 1;
            int b = (i ^ j) & 0xFF;
            int c = a * b - j;
            int d = (c << 2) | (j >> 1);
            
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            
            /* Function call in inner loop */
            int r = ext_func1(a);
            
            int e = d * r + c;
            int f = (e & b) | a;
            
            asm volatile("" : : "r"(e), "r"(f));
            
            sum += e + f;
            
            /* More computations to increase pressure */
            for (int k = 0; k < 4; k++) {
                int g = f * k + e;
                int h = (g << k) | (j >> k);
                asm volatile("" : : "r"(g), "r"(h));
                sum += g - h;
            }
        }
        
        /* Additional computations between outer loop iterations */
        int x = i * 7 + 3;
        int y = (x ^ 0x1234) * 11;
        asm volatile("" : : "r"(x), "r"(y));
        sum += x * y;
    }
    
    asm volatile("" : "+r"(sum));
    return sum;
}

/* Test function 4: 64-bit and vector operations */
v4si test_vector_ops(int seed) {
    v4si v1 = {seed, seed + 1, seed + 2, seed + 3};
    v4si v2 = {seed * 2, seed * 3, seed * 4, seed * 5};
    v4si v3 = {seed | 0xFF, seed & 0xF0, seed ^ 0xAA, ~seed};
    
    asm volatile("" : : "r"(v1), "r"(v2), "r"(v3));
    
    /* Vector operations */
    v4si t1 = v1 + v2;
    v4si t2 = v1 * v3;
    v4si t3 = t1 & v2;
    v4si t4 = t2 | v3;
    
    asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4));
    
    /* 64-bit operations */
    long long ll1 = (long long)seed * 123456789LL;
    long long ll2 = ll1 + 987654321LL;
    long long ll3 = ll1 ^ ll2;
    long long ll4 = ll2 * ll3;
    
    asm volatile("" : : "r"(ll1), "r"(ll2), "r"(ll3), "r"(ll4));
    
    /* External call with 64-bit */
    long long r = ext_func3(ll1);
    
    /* More mixed operations */
    v4si t5 = t4 + (v4si){r & 0xFF, (r >> 8) & 0xFF, (r >> 16) & 0xFF, (r >> 24) & 0xFF};
    long long ll5 = ll4 + r * 2;
    
    asm volatile("" : : "r"(t5), "r"(ll5));
    
    /* Combine results */
    v4si result = t5 + (v4si){ll5 & 0xFF, (ll5 >> 8) & 0xFF, 
                              (ll5 >> 16) & 0xFF, (ll5 >> 24) & 0xFF};
    
    asm volatile("" : "+r"(result));
    return result;
}

/* Test function 5: Extreme register pressure with all types */
double test_extreme_pressure(int seed) {
    volatile double final_result = 0.0;
    
    /* Create many variables of different types */
    int i1 = seed;
    int i2 = seed * 2;
    int i3 = seed + 100;
    int i4 = seed ^ 0x1234;
    long long ll1 = (long long)seed * 1000;
    long long ll2 = ll1 + 5000;
    double d1 = seed * 1.5;
    double d2 = d1 / 3.14;
    float f1 = seed * 0.25f;
    float f2 = f1 + 1.618f;
    
    /* Force all into registers */
    asm volatile("" : : "r"(i1), "r"(i2), "r"(i3), "r"(i4),
                       "r"(ll1), "r"(ll2), "r"(d1), "r"(d2),
                       "r"(f1), "r"(f2));
    
    /* Massive computation sequence */
    for (int i = 0; i < 20; i++) {
        i1 = i1 * i2 + i3;
        i2 = (i2 ^ i4) | i1;
        i3 = i3 * 3 - i2;
        i4 = (i4 << 2) & i1;
        
        ll1 = ll1 + ll2 * i;
        ll2 = ll1 ^ (ll2 << 3);
        
        d1 = d1 * 1.1 + d2;
        d2 = d2 / 1.01 - d1;
        
        f1 = f1 * 1.5f + f2;
        f2 = f2 - f1 * 0.5f;
        
        /* Force intermediate values to stay in registers */
        if (i % 5 == 0) {
            asm volatile("" : : "r"(i1), "r"(i2), "r"(i3), "r"(i4),
                               "r"(ll1), "r"(ll2), "r"(d1), "r"(d2),
                               "r"(f1), "r"(f2));
            
            /* Function call to clobber registers */
            int r = ext_func1(i1);
            i1 += r;
        }
    }
    
    /* Final combination */
    final_result = d1 + d2 + f1 + f2 + i1 + i2 + i3 + i4 + ll1 + ll2;
    asm volatile("" : "+r"(final_result));
    
    return final_result;
}

/* Dummy external functions */
int ext_func1(int x) { return x ^ 0x55AA55AA; }
double ext_func2(double x) { return x * 2.71828; }
long long ext_func3(long long x) { return x + 0x123456789ABCDEFLL; }

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : 1234;
    srand(seed);
    
    printf("Testing early rematerialization coverage...\n");
    
    /* Call all test functions to ensure they're not optimized away */
    int r1 = test_high_int_pressure(seed);
    double r2 = test_mixed_pressure((double)seed);
    long long r3 = test_nested_loops(seed % 50 + 10);
    v4si r4 = test_vector_ops(seed);
    double r5 = test_extreme_pressure(seed);
    
    /* Use results to prevent optimization */
    volatile int checksum = r1 + (int)r2 + (int)r3 + r4[0] + (int)r5;
    
    printf("Checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return 0;
}
