/* test_early_remat.c - Test program for GCC early rematerialization pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External non-inline functions to create register pressure points */
extern int ext_func1(int);
extern int ext_func2(int, int);
extern double ext_func3(double);
extern long long ext_func4(long long);

/* Dummy external functions to prevent inlining */
int ext_func1(int x) { return x ^ 0x12345678; }
int ext_func2(int x, int y) { return x * y + (x ^ y); }
double ext_func3(double x) { return x * 1.23456789; }
long long ext_func4(long long x) { return x * 3LL; }

/* Vector type to test different modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test function 1: High integer register pressure with complex expressions */
int test_integer_pressure(int seed) {
    volatile int result = 0;
    int a = seed * 3 + 1;
    int b = seed / 2 - 5;
    int c = seed ^ 0xABCD;
    int d = seed + 777;
    
    /* Long sequence of independent computations */
    int t1 = a * b + c - d;
    asm volatile("" : : "r"(t1)); /* Pin in register */
    
    int t2 = (a & b) | (c ^ d);
    int t3 = t1 * t2 + seed;
    
    /* Function call creates register pressure */
    int t4 = ext_func1(t3);
    
    int t5 = (t2 << 3) | (t4 >> 2);
    int t6 = t4 * t5 - t3;
    asm volatile("" : : "r"(t6));
    
    int t7 = ext_func2(t5, t6);
    
    int t8 = t7 ^ t6 ^ t5;
    int t9 = t8 * 31 + 17;
    int t10 = (t9 & 0xFF) | ((t9 >> 8) & 0xFF00);
    
    /* More computations to increase pressure */
    for (int i = 0; i < 8; i++) {
        t10 = t10 * 1103515245 + 12345;
        asm volatile("" : : "r"(t10));
    }
    
    result = t10;
    return result;
}

/* Test function 2: Mixed integer and floating-point pressure */
double test_mixed_pressure(double seed) {
    volatile double result = 0.0;
    double d1 = seed * 1.5;
    double d2 = seed / 3.0;
    int i1 = (int)(seed * 1000);
    int i2 = (int)(seed * 2000);
    
    /* Mix FP and integer ops */
    double d3 = d1 * d2 + seed;
    int i3 = i1 * i2 + (int)d3;
    
    asm volatile("" : : "r"(i3), "f"(d3)); /* Pin both types */
    
    /* Function call that uses both */
    double d4 = ext_func3(d3);
    int i4 = ext_func1(i3);
    
    /* More mixed computations */
    double d5 = d4 * (double)i4;
    int i5 = i4 ^ (int)d5;
    
    /* Nested loops with derived induction variables */
    for (int i = 0; i < 4; i++) {
        for (int j = i * 2; j < 8; j += 3) {
            d5 += (double)(i5 * j);
            i5 += i * j;
            asm volatile("" : : "r"(i5), "f"(d5));
        }
        /* Call between loop iterations */
        i5 = ext_func2(i5, i);
    }
    
    result = d5 + (double)i5;
    return result;
}

/* Test function 3: Long long and vector operations */
long long test_wide_types(long long seed) {
    volatile long long result = 0;
    long long ll1 = seed * 5LL;
    long long ll2 = seed + 1234567890123LL;
    
    /* Vector operations */
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3;
    
    /* Complex sequence with wide types */
    ll1 = ll1 * ll2 - seed;
    asm volatile("" : : "r"(ll1));
    
    ll2 = ext_func4(ll1);
    
    for (int i = 0; i < 16; i++) {
        v3 = v1 + v2;
        v1 = v2 * (v4si){i, i+1, i+2, i+3};
        v2 = v3 ^ v1;
        
        /* Use vector elements in scalar computations */
        ll1 += v2[0] + v2[1] + v2[2] + v2[3];
        asm volatile("" : : "r"(ll1));
        
        /* Function call in loop */
        if (i % 4 == 0) {
            ll2 = ext_func4(ll1);
        }
    }
    
    result = ll1 + ll2;
    return result;
}

/* Test function 4: Extreme register pressure with many temporaries */
double test_extreme_pressure(int iterations) {
    volatile double checksum = 0.0;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Create many unique temporary variables */
        double d0 = iter * 1.1;
        double d1 = d0 * 2.2;
        double d2 = d1 / 3.3;
        double d3 = d2 + 4.4;
        double d4 = d3 - 5.5;
        double d5 = d4 * 6.6;
        double d6 = d5 / 7.7;
        double d7 = d6 + 8.8;
        double d8 = d7 - 9.9;
        double d9 = d8 * 10.10;
        
        int i0 = iter * 3;
        int i1 = i0 + 1;
        int i2 = i1 * 2;
        int i3 = i2 ^ 0x1234;
        int i4 = i3 & 0xABCD;
        int i5 = i4 | 0x5678;
        int i6 = i5 << 2;
        int i7 = i6 >> 1;
        int i8 = i7 * 31;
        int i9 = i8 + 17;
        
        /* Pin all temporaries to force register allocation */
        asm volatile("" : : 
            "r"(i0), "r"(i1), "r"(i2), "r"(i3), "r"(i4),
            "r"(i5), "r"(i6), "r"(i7), "r"(i8), "r"(i9),
            "f"(d0), "f"(d1), "f"(d2), "f"(d3), "f"(d4),
            "f"(d5), "f"(d6), "f"(d7), "f"(d8), "f"(d9));
        
        /* Function calls between computations */
        d9 = ext_func3(d9);
        i9 = ext_func1(i9);
        
        /* Complex expression using all temporaries */
        checksum += d0 * i0 + d1 * i1 - d2 * i2 + d3 * i3 -
                   d4 * i4 + d5 * i5 - d6 * i6 + d7 * i7 -
                   d8 * i8 + d9 * i9;
        
        asm volatile("" : : "f"(checksum));
    }
    
    return checksum;
}

/* Main function that runs all tests */
int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : time(NULL);
    double total = 0.0;
    
    printf("Testing early rematerialization coverage...\n");
    
    /* Run test 1: Integer pressure */
    int r1 = test_integer_pressure(seed);
    printf("Test 1 result: %d\n", r1);
    total += r1;
    
    /* Run test 2: Mixed pressure */
    double r2 = test_mixed_pressure((double)seed);
    printf("Test 2 result: %f\n", r2);
    total += r2;
    
    /* Run test 3: Wide types */
    long long r3 = test_wide_types(seed);
    printf("Test 3 result: %lld\n", r3);
    total += (double)r3;
    
    /* Run test 4: Extreme pressure */
    double r4 = test_extreme_pressure(argc > 2 ? atoi(argv[2]) : 10);
    printf("Test 4 result: %f\n", r4);
    total += r4;
    
    /* Final volatile store to prevent optimization */
    volatile double final_result = total;
    printf("Total checksum: %f\n", final_result);
    
    return (int)final_result % 256;
}
