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
double ext_func3(double x) { return x * 3.141592653589793; }
long long ext_func4(long long x) { return x * 0xDEADBEEFCAFEBABEULL; }

/* Vector type to test different modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High register pressure with mixed integer operations */
int test_mixed_integer_pressure(int seed) {
    volatile int result = 0;
    int a = seed * 3;
    int b = seed + 7;
    int c = seed ^ 0x1234;
    int d = seed | 0xABCD;
    
    /* Force values into registers */
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
    
    /* Long sequence of independent computations */
    int t1 = a * b + c;
    int t2 = b & c | d;
    int t3 = (a << 3) ^ (b >> 2);
    int t4 = ext_func1(t1);  /* Function call creates pressure point */
    
    asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4));
    
    int t5 = t2 * t3 - t4;
    int t6 = t1 & t4 | t3;
    int t7 = ext_func2(t5, t6);  /* Another pressure point */
    
    int t8 = t5 ^ t6 ^ t7;
    int t9 = (t8 << 1) | (t7 >> 1);
    int t10 = t9 * 0x9E3779B9;
    
    asm volatile("" : : "r"(t5), "r"(t6), "r"(t7), "r"(t8), "r"(t9), "r"(t10));
    
    /* More computations with function calls in between */
    for (int i = 0; i < 4; i++) {
        t10 = ext_func1(t10 + i);
        t9 = t9 ^ (t10 * i);
        asm volatile("" : : "r"(t9), "r"(t10));
    }
    
    result = t9 + t10;
    return result;
}

/* Test 2: Floating point and mixed-mode operations */
double test_floating_pressure(double seed) {
    volatile double result = 0.0;
    double a = seed * 1.234;
    double b = seed + 5.678;
    float c = (float)seed * 2.5f;
    float d = (float)seed / 3.0f;
    
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
    
    double t1 = a * b + c;
    float t2 = c * d - (float)a;
    double t3 = ext_func3(t1);  /* Pressure point */
    
    asm volatile("" : : "r"(t1), "r"(t2), "r"(t3));
    
    /* Mix float and double operations */
    double t4 = t1 * (double)t2 + t3;
    float t5 = (float)t4 * t2;
    double t6 = ext_func3(t4);
    
    /* Long sequence to increase pressure */
    for (int i = 0; i < 8; i++) {
        t4 = t4 * 1.1 + i;
        t5 = t5 * 1.2f - i;
        asm volatile("" : : "r"(t4), "r"(t5));
        if (i % 3 == 0) {
            t6 = ext_func3(t6 + i);
        }
    }
    
    result = t4 + t5 + t6;
    return result;
}

/* Test 3: Nested loops with complex induction variables */
long long test_nested_loops(int N) {
    volatile long long result = 0;
    long long acc = 0;
    
    for (int i = 0; i < N; i++) {
        /* Complex induction variable for inner loop */
        for (int j = i * 2; j < N; j += 3) {
            int k = j ^ i;
            long long temp = (long long)i * j * k;
            
            /* Multiple intermediate computations */
            long long t1 = temp * 0x12345678;
            long long t2 = t1 ^ 0xABCDEF;
            long long t3 = ext_func4(t2);  /* Pressure point */
            
            asm volatile("" : : "r"(t1), "r"(t2), "r"(t3));
            
            /* More computations */
            long long t4 = t1 + t2 * t3;
            long long t5 = t4 >> (j & 0xF);
            long long t6 = ext_func4(t5);
            
            acc += t4 + t5 + t6;
            
            /* Additional pressure with function call */
            if ((j & 7) == 0) {
                acc = ext_func4(acc);
            }
        }
        
        /* Outer loop computations */
        long long outer_temp = acc * i;
        long long outer_t2 = ext_func4(outer_temp);
        acc = (acc + outer_temp) ^ outer_t2;
        
        asm volatile("" : : "r"(acc), "r"(outer_temp), "r"(outer_t2));
    }
    
    result = acc;
    return result;
}

/* Test 4: Vector operations for different modes */
int test_vector_operations(int seed) {
    v4si v1 = {seed, seed + 1, seed + 2, seed + 3};
    v4si v2 = {seed * 2, seed * 3, seed * 4, seed * 5};
    v4si v3 = {seed ^ 1, seed ^ 2, seed ^ 3, seed ^ 4};
    
    /* Multiple vector operations */
    v4si r1 = v1 + v2;
    v4si r2 = v1 * v3;
    v4si r3 = r1 & v2;
    v4si r4 = r2 | v3;
    
    /* Extract and use scalar elements to prevent elimination */
    int elems[4];
    elems[0] = r1[0] + r2[0];
    elems[1] = r3[1] * r4[1];
    elems[2] = ext_func1(r2[2]);  /* Pressure point */
    elems[3] = r4[3] ^ r1[3];
    
    asm volatile("" : : "r"(r1), "r"(r2), "r"(r3), "r"(r4));
    
    /* More vector computations */
    for (int i = 0; i < 8; i++) {
        r1 = r1 + r2;
        r2 = r2 * r3;
        r3 = r3 & r4;
        r4 = r4 | r1;
        
        /* Function call creates register pressure */
        if (i % 2 == 0) {
            elems[i % 4] = ext_func1(elems[i % 4] + i);
        }
        
        asm volatile("" : : "r"(r1), "r"(r2), "r"(r3), "r"(r4));
    }
    
    int result = elems[0] + elems[1] + elems[2] + elems[3];
    return result;
}

/* Test 5: Extreme register pressure with many live values */
long long test_extreme_pressure(int iterations) {
    volatile long long checksum = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Create many live variables */
        long long v1 = iter * 0x11111111;
        long long v2 = iter + 0x22222222;
        long long v3 = iter ^ 0x33333333;
        long long v4 = iter | 0x44444444;
        long long v5 = iter & 0x55555555;
        long long v6 = iter << 3;
        long long v7 = iter >> 2;
        long long v8 = v1 * v2;
        long long v9 = v3 + v4;
        long long v10 = v5 ^ v6;
        
        /* Force all into registers */
        asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4), "r"(v5),
                         "r"(v6), "r"(v7), "r"(v8), "r"(v9), "r"(v10));
        
        /* Complex computation graph */
        long long t1 = v1 * v2 + v3;
        long long t2 = v4 & v5 | v6;
        long long t3 = ext_func4(t1);  /* Major pressure point */
        
        long long t4 = v7 ^ v8 ^ v9;
        long long t5 = t2 * t3 - t4;
        long long t6 = ext_func4(t5);
        
        long long t7 = t1 + t4 * t6;
        long long t8 = t2 ^ t3 ^ t5;
        long long t9 = ext_func4(t7);
        
        asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4),
                         "r"(t5), "r"(t6), "r"(t7), "r"(t8), "r"(t9));
        
        /* Use results to prevent elimination */
        checksum += t1 + t3 + t5 + t7 + t9;
        
        /* Function call that clobbers registers */
        if (iter % 4 == 0) {
            checksum = ext_func4(checksum);
        }
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    int N = argc > 2 ? atoi(argv[2]) : 100;
    
    printf("Testing early rematerialization coverage...\n");
    
    /* Run all tests to exercise different patterns */
    int r1 = test_mixed_integer_pressure(seed);
    printf("Test 1 result: %d\n", r1);
    
    double r2 = test_floating_pressure(seed * 1.0);
    printf("Test 2 result: %f\n", r2);
    
    long long r3 = test_nested_loops(N);
    printf("Test 3 result: %lld\n", r3);
    
    int r4 = test_vector_operations(seed);
    printf("Test 4 result: %d\n", r4);
    
    long long r5 = test_extreme_pressure(N / 10);
    printf("Test 5 result: %lld\n", r5);
    
    /* Final volatile store to prevent optimization */
    volatile int final = r1 + (int)r2 + (int)r3 + r4 + (int)r5;
    printf("Final checksum: %d\n", final);
    
    return 0;
}
