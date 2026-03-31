/* test_early_remat.c - Test program for GCC early rematerialization pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External non-inline functions to force register pressure */
extern int external_func1(int);
extern int external_func2(int, int);
extern double external_func3(double);
extern long long external_func4(long long);

/* Dummy implementations to satisfy linker */
int external_func1(int x) { return x ^ 0x1234; }
int external_func2(int x, int y) { return (x * y) & 0xFFFF; }
double external_func3(double x) { return x * 1.2345; }
long long external_func4(long long x) { return x + 0x12345678; }

/* Vector type to test different modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High register pressure with mixed integer operations */
int test1_high_pressure_int(int seed) {
    volatile int result = 0;
    int a = seed * 3;
    int b = seed + 7;
    int c = seed ^ 0x55;
    int d = seed >> 2;
    
    /* Long sequence of independent computations */
    int t1 = a * b + c;
    asm volatile("" : : "r"(t1)); /* Force t1 to be in register */
    
    int t2 = (b & c) | d;
    int t3 = t1 ^ t2;
    asm volatile("" : : "r"(t3));
    
    int t4 = external_func1(t2); /* Function call creates pressure point */
    
    int t5 = t3 * t4 - a;
    int t6 = (t5 << 3) | (t4 >> 2);
    asm volatile("" : : "r"(t6));
    
    int t7 = external_func2(t5, t6);
    
    int t8 = t7 + t6 * 17;
    int t9 = (t8 & 0xFF) ^ t7;
    asm volatile("" : : "r"(t9));
    
    int t10 = t9 * 3 + t8 / 5;
    int t11 = (t10 ^ 0xAA) & 0x7F;
    
    /* More computations to increase pressure */
    for (int i = 0; i < 8; i++) {
        t11 = t11 * 2 + i;
        asm volatile("" : : "r"(t11));
    }
    
    result = t11;
    return result;
}

/* Test 2: Mixed floating-point and integer operations */
double test2_mixed_fp_int(double seed) {
    volatile double result = 0.0;
    double d1 = seed * 1.5;
    double d2 = seed + 2.71828;
    int i1 = (int)(seed * 1000);
    int i2 = (int)(seed * 1000) ^ 0x1234;
    
    asm volatile("" : : "r"(i1), "r"(i2));
    
    double d3 = d1 * d2 + 3.14159;
    int i3 = i1 * i2 + (int)d3;
    
    asm volatile("" : : "r"(i3));
    
    double d4 = external_func3(d3); /* FP function call */
    
    int i4 = external_func1(i3);
    double d5 = d4 * 2.0 - d1;
    
    asm volatile("" : : "r"(i4));
    
    /* Mix operations */
    for (int i = 0; i < 4; i++) {
        d5 = d5 * (1.0 + i * 0.1);
        i4 = i4 * 2 + i;
        asm volatile("" : : "r"(i4), "r"(d5));
    }
    
    result = d5 + i4;
    return result;
}

/* Test 3: Nested loops with complex induction variables */
long long test3_nested_loops(int N) {
    volatile long long total = 0;
    
    for (int i = 0; i < N; i++) {
        /* Complex induction variable */
        for (int j = i * 2; j < N; j += 3) {
            int k = j * 3 - i;
            
            /* Multiple independent computations in loop body */
            long long val1 = (long long)i * j * k;
            long long val2 = external_func4(val1);
            
            asm volatile("" : : "r"(val1), "r"(val2));
            
            int temp = (i ^ j) & k;
            long long val3 = val2 + temp * 7;
            
            /* Function call inside inner loop */
            int val4 = external_func1(temp);
            
            asm volatile("" : : "r"(val3), "r"(val4));
            
            total += val3 * val4;
            
            /* Additional computation to increase pressure */
            for (int m = 0; m < 2; m++) {
                long long val5 = total + m * 11;
                int val6 = external_func2(m, val4);
                asm volatile("" : : "r"(val5), "r"(val6));
                total = val5 ^ val6;
            }
        }
    }
    
    return total;
}

/* Test 4: Vector operations and 64-bit integers */
v4si test4_vector_ops(int base) {
    v4si v1 = {base, base + 1, base + 2, base + 3};
    v4si v2 = {base + 4, base + 5, base + 6, base + 7};
    
    /* Chain of vector operations */
    v4si v3 = v1 + v2;
    v4si v4 = v3 * v2;
    v4si v5 = v4 & v1;
    
    asm volatile("" : : "r"(v3), "r"(v4), "r"(v5));
    
    /* Mix with scalar operations */
    long long ll1 = (long long)base * 1000000;
    long long ll2 = external_func4(ll1);
    
    v4si v6 = v5 + (v4si){ll2 & 0xFF, (ll2 >> 8) & 0xFF, 
                          (ll2 >> 16) & 0xFF, (ll2 >> 24) & 0xFF};
    
    asm volatile("" : : "r"(ll1), "r"(ll2), "r"(v6));
    
    /* More computations */
    for (int i = 0; i < 4; i++) {
        ll1 = ll1 * 3 + i;
        v6[i] = v6[i] * 2 + (int)(ll1 & 0xFF);
        asm volatile("" : : "r"(ll1));
    }
    
    return v6;
}

/* Test 5: Extreme register pressure with many live values */
int test5_extreme_pressure(int iterations) {
    volatile int checksum = 0;
    
    /* Declare many variables to force register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    
    for (int i = 0; i < iterations; i++) {
        /* Keep all variables live through complex computations */
        v1 = v1 * v2 + v3;
        v2 = v2 ^ v4 | v5;
        v3 = v3 + v6 - v7;
        v4 = v4 * v8 / (v9 + 1);
        v5 = v5 & v10 ^ v11;
        
        asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4), "r"(v5));
        
        v6 = external_func1(v6);
        v7 = v7 * 3 + v12;
        v8 = v8 ^ v13 & v14;
        v9 = v9 + v15 - v1;
        v10 = v10 * v2 / (v3 + 1);
        
        asm volatile("" : : "r"(v6), "r"(v7), "r"(v8), "r"(v9), "r"(v10));
        
        v11 = external_func2(v11, v12);
        v12 = v12 + v4 * v5;
        v13 = v13 ^ v6 | v7;
        v14 = v14 * v8 - v9;
        v15 = v15 & v10 ^ v11;
        
        asm volatile("" : : "r"(v11), "r"(v12), "r"(v13), "r"(v14), "r"(v15));
        
        checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                   v11 + v12 + v13 + v14 + v15;
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    int N = argc > 2 ? atoi(argv[2]) : 100;
    
    printf("Testing early rematerialization coverage...\n");
    
    /* Run all tests to exercise different patterns */
    int r1 = test1_high_pressure_int(seed);
    printf("Test1 result: %d\n", r1);
    
    double r2 = test2_mixed_fp_int(seed * 1.234);
    printf("Test2 result: %f\n", r2);
    
    long long r3 = test3_nested_loops(N);
    printf("Test3 result: %lld\n", r3);
    
    v4si r4 = test4_vector_ops(seed);
    printf("Test4 result: %d %d %d %d\n", r4[0], r4[1], r4[2], r4[3]);
    
    int r5 = test5_extreme_pressure(N / 10);
    printf("Test5 result: %d\n", r5);
    
    /* Final volatile store to prevent optimization */
    volatile int final_result = r1 + (int)r2 + (int)r3 + r4[0] + r5;
    
    return final_result != 0 ? 0 : 1;
}
