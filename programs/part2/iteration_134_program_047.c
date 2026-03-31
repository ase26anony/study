/* test_early_remat.c - Test program for GCC early rematerialization pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External non-inline functions to create register pressure points */
extern int external_func1(int);
extern int external_func2(int, int);
extern double external_func3(double);
extern long long external_func4(long long);

/* Dummy implementations to satisfy linker */
int external_func1(int x) { return x ^ 0x55AA55AA; }
int external_func2(int x, int y) { return (x * y) ^ 0x12345678; }
double external_func3(double x) { return x * 1.234567; }
long long external_func4(long long x) { return x * 0xDEADBEEF; }

/* Vector type to test different modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High integer register pressure with complex expressions */
int test_high_int_pressure(int seed) {
    volatile int result = 0;
    int a = seed * 3 + 7;
    int b = seed / 2 - 5;
    
    /* Long sequence of independent computations */
    int t1 = a * b + seed;
    asm volatile("" : : "r"(t1)); /* Force t1 to be in register */
    
    int t2 = (a & 0xFF) | (b << 8);
    int t3 = t1 ^ t2;
    
    /* Function call creates register pressure */
    int t4 = external_func1(t3);
    
    int t5 = t4 * 31 + 17;
    int t6 = t5 - (seed >> 3);
    asm volatile("" : : "r"(t6));
    
    int t7 = external_func2(t6, t4);
    int t8 = t7 & 0xAAAAAAAA;
    int t9 = t8 | 0x55555555;
    
    int t10 = t9 * 73 - 29;
    asm volatile("" : : "r"(t10));
    
    int t11 = t10 ^ t7 ^ t4;
    int t12 = external_func1(t11);
    
    int t13 = t12 + (t10 << 2);
    int t14 = t13 - (t7 >> 1);
    asm volatile("" : : "r"(t14));
    
    int t15 = external_func2(t14, t13);
    int t16 = t15 * 3 / 2;
    
    /* More computations to increase pressure */
    for (int i = 0; i < 8; i++) {
        t16 = t16 + external_func1(t16 + i);
        asm volatile("" : : "r"(t16));
    }
    
    result = t16;
    return result;
}

/* Test 2: Mixed integer and floating-point operations */
double test_mixed_types(int seed) {
    volatile double result = 0.0;
    double d1 = seed * 1.234;
    double d2 = seed / 3.14159;
    
    int i1 = seed * 7;
    int i2 = seed + 12345;
    
    /* Mix operations */
    double t1 = d1 * d2 + i1;
    asm volatile("" : : "r"(i1), "r"(i2));
    
    int i3 = i1 & i2 | (seed << 3);
    double t2 = external_func3(t1);
    
    double t3 = t2 * 2.71828 - d1;
    int i4 = external_func1(i3);
    
    double t4 = t3 + i4;
    asm volatile("" : : "r"(i4));
    
    /* More mixing */
    for (int i = 0; i < 4; i++) {
        i4 = external_func2(i4, i);
        t4 = external_func3(t4) + i4;
        asm volatile("" : : "r"(i4), "r"(t4));
    }
    
    result = t4;
    return result;
}

/* Test 3: Nested loops with complex induction */
long long test_nested_loops(int N) {
    volatile long long result = 0;
    long long acc = 0;
    
    for (int i = 0; i < N; i++) {
        /* Complex induction variable */
        for (int j = i * 2; j < N; j += 3) {
            int k = j + (i << 1);
            
            /* Independent computations in inner loop */
            long long t1 = external_func4(k);
            long long t2 = external_func4(j);
            
            int i1 = external_func1(k);
            int i2 = external_func2(j, i);
            
            asm volatile("" : : "r"(t1), "r"(t2), "r"(i1), "r"(i2));
            
            long long t3 = t1 ^ t2;
            int i3 = i1 | i2;
            
            acc += t3 + i3;
            
            /* Function call in inner loop */
            if (j % 5 == 0) {
                i3 = external_func1(i3);
                asm volatile("" : : "r"(i3));
            }
        }
        
        /* Additional computation between loops */
        int tmp = external_func2(i, N - i);
        acc += external_func4(tmp);
        asm volatile("" : : "r"(tmp));
    }
    
    result = acc;
    return result;
}

/* Test 4: Vector operations with mixed modes */
int test_vector_ops(int seed) {
    volatile int result = 0;
    
    /* Create vector values */
    v4si v1 = {seed, seed + 1, seed + 2, seed + 3};
    v4si v2 = {seed * 2, seed * 3, seed * 4, seed * 5};
    
    /* Scalar computations mixed with vector */
    int s1 = seed * 7 + 11;
    int s2 = seed / 3 - 5;
    
    /* Vector operations */
    v4si v3 = v1 + v2;
    v4si v4 = v1 * v2;
    
    asm volatile("" : : "r"(s1), "r"(s2));
    
    /* More scalar computations */
    for (int i = 0; i < 4; i++) {
        s1 = external_func2(s1, s2 + i);
        s2 = external_func1(s1);
        
        /* Access vector elements */
        int velem = v3[i] + v4[i % 2];
        s1 += external_func1(velem);
        
        asm volatile("" : : "r"(s1), "r"(s2), "r"(velem));
    }
    
    /* Final mix */
    v4si v5 = v3 - v4;
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += v5[i] + s1;
        s1 = external_func2(s1, i);
    }
    
    result = sum + s2;
    return result;
}

/* Test 5: Extreme register pressure with many live values */
double test_extreme_pressure(int iterations) {
    volatile double result = 0.0;
    
    /* Many independent variables */
    double d1 = 1.1, d2 = 2.2, d3 = 3.3, d4 = 4.4, d5 = 5.5;
    int i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    long long l1 = 100, l2 = 200, l3 = 300, l4 = 400, l5 = 500;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Complex expressions creating many temporaries */
        double td1 = d1 * d2 + d3 - d4 / d5;
        int ti1 = (i1 & i2) | (i3 ^ i4) + i5;
        long long tl1 = l1 * l2 + l3 - l4 / (l5 + 1);
        
        asm volatile("" : : "r"(td1), "r"(ti1), "r"(tl1));
        
        /* Function calls between dependent computations */
        d1 = external_func3(td1);
        i1 = external_func1(ti1);
        l1 = external_func4(tl1);
        
        double td2 = d2 * d3 + d4 - d5 / d1;
        int ti2 = (i2 & i3) | (i4 ^ i5) + i1;
        long long tl2 = l2 * l3 + l4 - l5 / (l1 + 1);
        
        asm volatile("" : : "r"(td2), "r"(ti2), "r"(tl2));
        
        d2 = external_func3(td2);
        i2 = external_func2(ti2, ti1);
        l2 = external_func4(tl2);
        
        /* And so on... rotating through all variables */
        double td3 = d3 * d4 + d5 - d1 / d2;
        int ti3 = (i3 & i4) | (i5 ^ i1) + i2;
        long long tl3 = l3 * l4 + l5 - l1 / (l2 + 1);
        
        asm volatile("" : : "r"(td3), "r"(ti3), "r"(tl3));
        
        d3 = external_func3(td3);
        i3 = external_func1(ti3);
        l3 = external_func4(tl3);
        
        /* Continue pattern for all variables */
        if (iter % 10 == 0) {
            /* Extra pressure point */
            result += d1 + d2 + d3 + d4 + d5 + i1 + i2 + i3 + i4 + i5;
        }
    }
    
    result += d1 + d2 + d3 + d4 + d5;
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
    
    long long r3 = test_nested_loops(seed % 20 + 5);
    printf("Test 3 result: %lld\n", r3);
    
    int r4 = test_vector_ops(seed);
    printf("Test 4 result: %d\n", r4);
    
    double r5 = test_extreme_pressure(seed % 10 + 3);
    printf("Test 5 result: %f\n", r5);
    
    /* Final volatile store to prevent optimization */
    volatile int final_check = r1 + (int)r2 + (int)r3 + r4 + (int)r5;
    printf("Final checksum: %d\n", final_check);
    
    return 0;
}
