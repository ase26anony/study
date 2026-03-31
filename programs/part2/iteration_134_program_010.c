/* test_early_remat.c - Test program for GCC early rematerialization pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External non-inline functions to create register pressure points */
extern int ext_func1(int x);
extern double ext_func2(double x);
extern long long ext_func3(long long x);

/* Dummy external functions to prevent inlining */
int ext_func1(int x) { return x ^ 0x55AA55AA; }
double ext_func2(double x) { return x * 1.23456789; }
long long ext_func3(long long x) { return x + 0x123456789ABCDEFLL; }

/* Vector type to test different modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High integer register pressure with complex expressions */
int test_high_int_pressure(int seed) {
    volatile int result = 0;
    int a = seed * 3 + 1;
    int b = seed / 2 - 5;
    int c = seed + 0x7FFF;
    int d = seed ^ 0xABCD;
    int e = seed | 0x1234;
    int f = seed & 0xF0F0;
    int g = seed << 3;
    int h = seed >> 2;
    
    /* Force values into registers with inline asm */
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
    
    /* Long sequence of independent computations */
    int t1 = a * b + c - d;
    int t2 = (e & f) | (g ^ h);
    int t3 = t1 * t2 / (a + 1);
    int t4 = (b << 2) + (c >> 1) - d;
    
    /* Function call creates register pressure */
    t1 = ext_func1(t1);
    
    asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4));
    
    int t5 = t1 * t3 + t2 * t4;
    int t6 = (t1 ^ t2) & (t3 | t4);
    int t7 = t5 << (t6 & 0xF);
    int t8 = t6 >> (t5 & 0x7);
    
    /* More function calls */
    t5 = ext_func1(t5);
    t6 = ext_func1(t6);
    
    int t9 = t7 * t8 - t5 + t6;
    int t10 = (t7 & t8) | (t5 ^ t6);
    int t11 = t9 * 3 + t10 / 2;
    int t12 = (t9 << 1) ^ (t10 >> 1);
    
    asm volatile("" : : "r"(t9), "r"(t10), "r"(t11), "r"(t12));
    
    /* Final computation mixing everything */
    result = t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 + t11 + t12;
    
    /* Prevent optimization */
    asm volatile("" : "+r"(result));
    return result;
}

/* Test 2: Mixed float/double operations */
double test_mixed_float_pressure(double seed) {
    volatile double result = 0.0;
    double a = seed * 1.5;
    double b = seed / 2.0;
    double c = seed + 3.14159;
    double d = seed - 2.71828;
    float e = (float)seed * 0.5f;
    float f = (float)seed + 1.0f;
    float g = (float)seed / 3.0f;
    float h = (float)seed - 4.0f;
    
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
    
    /* Mix float and double computations */
    double t1 = a * b + c - d;
    float t2 = e * f + g - h;
    double t3 = t1 * (double)t2;
    float t4 = (float)t1 * g;
    
    /* Function call with double */
    t1 = ext_func2(t1);
    
    asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4));
    
    double t5 = t3 * 2.5 + t1;
    float t6 = t4 * 1.5f + (float)t2;
    double t7 = t5 / 1.234 + (double)t6;
    float t8 = t6 * 0.789f - (float)t5;
    
    /* More function calls */
    t5 = ext_func2(t5);
    t7 = ext_func2(t7);
    
    asm volatile("" : : "r"(t5), "r"(t6), "r"(t7), "r"(t8));
    
    result = t1 + t3 + t5 + t7 + (double)(t2 + t4 + t6 + t8);
    asm volatile("" : "+r"(result));
    return result;
}

/* Test 3: Nested loops with complex induction */
long long test_nested_loops(int N) {
    volatile long long total = 0;
    
    for (int i = 0; i < N; i++) {
        /* Complex induction variable */
        for (int j = i * 2; j < N; j += 3) {
            /* Inner loop with register pressure */
            int a = i * 3 + 1;
            int b = j * 2 - 5;
            int c = (i ^ j) + 0x7FFF;
            int d = (i & j) | 0xABCD;
            
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            
            int t1 = a * b + c - d;
            int t2 = (a ^ b) & (c | d);
            int t3 = t1 << (t2 & 0xF);
            int t4 = t2 >> (t1 & 0x7);
            
            /* Function call in inner loop */
            t1 = ext_func1(t1);
            
            int t5 = t3 * t4 - t1;
            int t6 = (t3 & t4) | t1;
            
            asm volatile("" : : "r"(t5), "r"(t6));
            
            total += (long long)t5 * t6;
            
            /* Additional computation to increase pressure */
            if ((i + j) % 7 == 0) {
                int extra = ext_func1(i + j);
                total += extra;
            }
        }
        
        /* Outer loop computation */
        if (i % 5 == 0) {
            long long outer_val = ext_func3(i);
            total ^= outer_val;
        }
    }
    
    asm volatile("" : "+r"(total));
    return total;
}

/* Test 4: Vector operations with mixed modes */
v4si test_vector_ops(v4si seed) {
    v4si a = seed + (v4si){1, 2, 3, 4};
    v4si b = seed * (v4si){2, 3, 4, 5};
    v4si c = seed & (v4si){0xFF, 0xFF00, 0xFF0000, 0xFF000000};
    v4si d = seed | (v4si){0x11, 0x22, 0x33, 0x44};
    
    /* Scalar extractions to force different modes */
    int a0 = a[0];
    int b1 = b[1];
    int c2 = c[2];
    int d3 = d[3];
    
    asm volatile("" : : "r"(a0), "r"(b1), "r"(c2), "r"(d3));
    
    /* Mix vector and scalar operations */
    v4si t1 = a * b + c;
    v4si t2 = b & c | d;
    int s1 = a0 * b1 + c2 - d3;
    int s2 = (a0 ^ b1) & (c2 | d3);
    
    /* Function call with scalar */
    s1 = ext_func1(s1);
    
    asm volatile("" : : "r"(t1), "r"(t2), "r"(s1), "r"(s2));
    
    v4si t3 = t1 * (v4si){s1, s2, s1, s2};
    v4si t4 = t2 & (v4si){s2, s1, s2, s1};
    int s3 = s1 * 3 + s2 / 2;
    int s4 = (s1 << 2) ^ (s2 >> 1);
    
    /* More computations */
    v4si result = t3 + t4 + (v4si){s3, s4, s3, s4};
    
    /* Prevent optimization */
    asm volatile("" : : "r"(result));
    return result;
}

/* Test 5: Extreme register pressure with all types */
long long test_extreme_pressure(int seed) {
    volatile long long checksum = 0;
    
    /* Use many variables of different types */
    int i1 = seed * 2;
    int i2 = seed + 0x1234;
    int i3 = seed ^ 0xABCD;
    long long ll1 = (long long)seed * 1000000LL;
    long long ll2 = (long long)seed + 0x123456789LL;
    double d1 = (double)seed * 1.234567;
    double d2 = (double)seed / 3.14159;
    float f1 = (float)seed * 0.12345f;
    float f2 = (float)seed + 2.71828f;
    
    /* Force all into registers */
    asm volatile("" : : "r"(i1), "r"(i2), "r"(i3), "r"(ll1), 
                  "r"(ll2), "r"(d1), "r"(d2), "r"(f1), "r"(f2));
    
    /* Complex computation sequence */
    for (int i = 0; i < 10; i++) {
        i1 = i1 * 3 + i;
        i2 = (i2 ^ i1) + i;
        i3 = i3 & i2 | i1;
        
        ll1 = ll1 * 2 + i;
        ll2 = ll2 ^ (ll1 << 3);
        
        d1 = d1 * 1.1 + (double)i;
        d2 = d2 / 1.01 - (double)i;
        
        f1 = f1 * 1.05f + (float)i;
        f2 = f2 - 0.95f * (float)i;
        
        /* Frequent function calls */
        if (i % 3 == 0) {
            i1 = ext_func1(i1);
            ll1 = ext_func3(ll1);
        }
        if (i % 4 == 0) {
            d1 = ext_func2(d1);
        }
        
        /* Force register retention */
        asm volatile("" : : "r"(i1), "r"(i2), "r"(i3), "r"(ll1),
                      "r"(ll2), "r"(d1), "r"(d2), "r"(f1), "r"(f2));
    }
    
    /* Final mixing */
    checksum = (long long)i1 + i2 + i3 + ll1 + ll2 + 
               (long long)d1 + (long long)d2 + 
               (long long)f1 + (long long)f2;
    
    asm volatile("" : "+r"(checksum));
    return checksum;
}

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    int N = argc > 2 ? atoi(argv[2]) : 100;
    
    printf("Testing early rematerialization coverage...\n");
    
    /* Run all tests to trigger different patterns */
    int r1 = test_high_int_pressure(seed);
    printf("Test 1 result: %d\n", r1);
    
    double r2 = test_mixed_float_pressure((double)seed);
    printf("Test 2 result: %f\n", r2);
    
    long long r3 = test_nested_loops(N);
    printf("Test 3 result: %lld\n", r3);
    
    v4si vec_seed = {seed, seed+1, seed+2, seed+3};
    v4si r4 = test_vector_ops(vec_seed);
    printf("Test 4 result: [%d, %d, %d, %d]\n", r4[0], r4[1], r4[2], r4[3]);
    
    long long r5 = test_extreme_pressure(seed);
    printf("Test 5 result: %lld\n", r5);
    
    /* Final checksum to prevent optimization */
    volatile long long final = r1 + (long long)r2 + r3 + 
                               r4[0] + r4[1] + r4[2] + r4[3] + r5;
    printf("Final checksum: %lld\n", final);
    
    return 0;
}
