/* test_early_remat.c - Test program for GCC early rematerialization pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External non-inline functions to create register pressure points */
extern int ext_func1(int);
extern int ext_func2(int, int);
extern double ext_func3(double);
extern long long ext_func4(long long);

/* Dummy external functions */
int ext_func1(int x) { return x ^ 0x55AA55AA; }
int ext_func2(int x, int y) { return (x * y) ^ 0x12345678; }
double ext_func3(double x) { return x * 1.234567; }
long long ext_func4(long long x) { return x + 0xDEADBEEF; }

/* Vector type to test different modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High integer register pressure with complex expressions */
int test_high_int_pressure(int seed) {
    volatile int result = 0;
    int a = seed * 3 + 1;
    int b = seed / 2 - 5;
    int c = seed ^ 0x1234;
    int d = seed + 777;
    
    /* Long sequence of independent computations */
    int t1 = a * b + c - d;
    asm volatile("" : : "r"(t1)); /* Force t1 into register */
    
    int t2 = (b << 3) | (c >> 2);
    int t3 = ext_func1(t2); /* Function call creates pressure */
    
    int t4 = t1 ^ t3;
    int t5 = (a & b) | (c & d);
    asm volatile("" : : "r"(t4), "r"(t5));
    
    int t6 = t4 * t5 - 12345;
    int t7 = ext_func2(t6, seed);
    
    int t8 = (t7 << 1) + (t6 >> 1);
    int t9 = t8 * 3 / 2;
    asm volatile("" : : "r"(t8), "r"(t9));
    
    int t10 = t9 ^ t7 ^ t6;
    int t11 = (t10 + 999) * 888;
    
    /* More computations to increase pressure */
    for (int i = 0; i < 8; i++) {
        t11 = t11 + (a << i) - (b >> i);
        asm volatile("" : : "r"(t11));
    }
    
    result = t11;
    return result;
}

/* Test 2: Mixed integer and floating point operations */
double test_mixed_types(int seed) {
    volatile double result = 0.0;
    double d1 = seed * 1.234;
    double d2 = seed / 3.456;
    float f1 = seed * 0.789f;
    float f2 = seed + 123.456f;
    
    /* Mix operations */
    double t1 = d1 * d2 + f1 * f2;
    asm volatile("" : : "r"(t1));
    
    int i1 = (int)d1 ^ (int)d2;
    double t2 = ext_func3(t1);
    
    float t3 = f1 + f2 - (float)t2;
    asm volatile("" : : "r"(t3));
    
    double t4 = t2 * 2.0 - t1;
    int i2 = ext_func1(i1);
    
    /* More mixed computations */
    for (int i = 0; i < 4; i++) {
        t4 = t4 + (d1 * i) - (d2 / (i + 1));
        i2 = i2 ^ (i1 << i);
        asm volatile("" : : "r"(t4), "r"(i2));
    }
    
    result = t4 + i2;
    return result;
}

/* Test 3: Nested loops with complex induction variables */
long long test_nested_loops(int N) {
    volatile long long sum = 0;
    
    for (int i = 0; i < N; i++) {
        /* Complex induction variable for inner loop */
        for (int j = i * 2; j < N; j += 3) {
            /* Multiple independent computations in loop body */
            long long a = i * 1000LL + j;
            long long b = (i ^ j) * 777LL;
            asm volatile("" : : "r"(a), "r"(b));
            
            long long c = a * b - 123456789LL;
            long long d = ext_func4(c);
            
            long long e = (c << 2) | (d >> 3);
            sum += e;
            
            /* Additional pressure inside loop */
            for (int k = 0; k < 2; k++) {
                e = e + (a << k) - (b >> k);
                asm volatile("" : : "r"(e));
            }
        }
        
        /* Function call between loop iterations */
        if (i % 7 == 0) {
            sum ^= ext_func1(i);
        }
    }
    
    return sum;
}

/* Test 4: Vector operations and 64-bit integers */
long long test_vector_and_64bit(int seed) {
    v4si v1 = {seed, seed + 1, seed + 2, seed + 3};
    v4si v2 = {seed * 2, seed * 3, seed * 4, seed * 5};
    
    /* Vector operations */
    v4si v3 = v1 + v2;
    v4si v4 = v1 * v2;
    asm volatile("" : : "r"(v3), "r"(v4));
    
    /* 64-bit operations */
    long long ll1 = (long long)seed * 1000000000LL;
    long long ll2 = ext_func4(ll1);
    
    long long ll3 = ll1 ^ ll2;
    long long ll4 = (ll1 << 13) | (ll2 >> 11);
    asm volatile("" : : "r"(ll3), "r"(ll4));
    
    /* Mix with vector elements */
    int elem1 = v3[0] ^ v4[1];
    long long ll5 = ll3 + elem1 * 100LL;
    
    /* More computations */
    for (int i = 0; i < 6; i++) {
        ll5 = ll5 + (ll1 >> i) - (ll2 << i);
        elem1 = elem1 ^ (v3[i % 4] & v4[(i + 1) % 4]);
        asm volatile("" : : "r"(ll5), "r"(elem1));
    }
    
    return ll5 + elem1;
}

/* Test 5: Extreme register pressure with many temporaries */
int test_extreme_pressure(int seed) {
    /* Create many unique temporary variables */
    int a = seed;
    int b = a * 3; asm volatile("" : : "r"(b));
    int c = b + 7; asm volatile("" : : "r"(c));
    int d = c ^ 0xABCD; asm volatile("" : : "r"(d));
    int e = d * 11; 
    int f = ext_func1(e); asm volatile("" : : "r"(f));
    int g = f - 123; asm volatile("" : : "r"(g));
    int h = g << 2; 
    int i = h | 0xFF; asm volatile("" : : "r"(i));
    int j = i * 3 / 2;
    int k = ext_func2(j, seed); asm volatile("" : : "r"(k));
    int l = k ^ j ^ i;
    int m = l + 999; asm volatile("" : : "r"(m));
    int n = m * 888;
    int o = n >> 3; asm volatile("" : : "r"(o));
    int p = o & 0x7F;
    int q = p * 17; 
    int r = ext_func1(q); asm volatile("" : : "r"(r));
    int s = r + 54321;
    int t = s ^ 0x12345678; asm volatile("" : : "r"(t));
    
    /* Use all temporaries in final computation */
    int result = a + b - c + d - e + f - g + h - i + j 
                 - k + l - m + n - o + p - q + r - s + t;
    
    return result;
}

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : time(NULL);
    
    printf("Testing early rematerialization coverage...\n");
    
    /* Run all tests to exercise different patterns */
    int r1 = test_high_int_pressure(seed);
    printf("Test 1 result: %d\n", r1);
    
    double r2 = test_mixed_types(seed);
    printf("Test 2 result: %f\n", r2);
    
    long long r3 = test_nested_loops(seed % 20 + 10);
    printf("Test 3 result: %lld\n", r3);
    
    long long r4 = test_vector_and_64bit(seed);
    printf("Test 4 result: %lld\n", r4);
    
    int r5 = test_extreme_pressure(seed);
    printf("Test 5 result: %d\n", r5);
    
    /* Final checksum to prevent optimization */
    volatile int final = r1 + (int)r2 + (int)r3 + (int)r4 + r5;
    printf("Final checksum: %d\n", final);
    
    return 0;
}
