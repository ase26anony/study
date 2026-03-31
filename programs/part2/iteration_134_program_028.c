/* test_early_remat.c - Comprehensive test for GCC early rematerialization pass */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External non-inline functions to create register pressure points */
extern int external_func1(int);
extern int external_func2(int, int);
extern double external_func3(double);
extern long long external_func4(long long);

/* Dummy implementations to satisfy linker */
int external_func1(int x) { return x ^ 0x1234; }
int external_func2(int x, int y) { return x * y + 0x5678; }
double external_func3(double x) { return x * 1.2345; }
long long external_func4(long long x) { return x + 0x9ABCDEF0; }

/* Vector type to engage different machine modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High integer register pressure with complex expressions */
int test_integer_pressure(int seed) {
    volatile int result = 0;
    int a = seed * 3 + 1;
    int b = seed / 2 - 5;
    int c = seed + 0x123456;
    int d = seed ^ 0x789ABC;
    
    /* Long sequence of independent computations */
    int t1 = a * b + c - d;
    asm volatile("" : : "r"(t1)); /* Pin in register */
    
    int t2 = (a & b) | (c ^ d);
    int t3 = t1 * t2 - a + b;
    asm volatile("" : : "r"(t3));
    
    int t4 = external_func1(t2); /* Function call creates pressure */
    
    int t5 = (t1 << 3) | (t3 >> 2);
    int t6 = t4 * t5 + 0xABCD;
    asm volatile("" : : "r"(t6));
    
    int t7 = external_func2(t3, t4);
    
    int t8 = t5 ^ t6 ^ t7;
    int t9 = t8 * 0x1234 - t1;
    asm volatile("" : : "r"(t9));
    
    int t10 = (t2 + t3) * (t4 - t5);
    int t11 = t10 / (t6 + 1) | t7;
    
    /* More computations to increase pressure */
    for (int i = 0; i < 8; i++) {
        t11 = t11 * 3 + i;
        asm volatile("" : : "r"(t11));
        t11 = external_func1(t11);
    }
    
    result = t9 + t11;
    return result;
}

/* Test 2: Mixed floating-point and integer pressure */
double test_mixed_pressure(double seed) {
    volatile double result = 0.0;
    double f1 = seed * 1.5;
    double f2 = seed / 3.14159;
    int i1 = (int)seed * 2;
    int i2 = (int)(seed * 100) & 0xFFF;
    
    asm volatile("" : : "r"(i1), "r"(i2));
    
    double f3 = f1 * f2 + (double)i1;
    asm volatile("" : : "f"(f3)); /* Pin floating register */
    
    double f4 = external_func3(f2);
    
    int i3 = i1 * i2 + (int)f1;
    int i4 = external_func2(i3, (int)f3);
    
    double f5 = f3 * f4 - (double)i4;
    asm volatile("" : : "f"(f5));
    
    /* More mixed computations */
    for (int i = 0; i < 6; i++) {
        f5 = f5 * 1.1 + (double)i;
        i4 = i4 * 2 - i;
        asm volatile("" : : "f"(f5), "r"(i4));
        f5 = external_func3(f5);
    }
    
    result = f5 + (double)i4;
    return result;
}

/* Test 3: Nested loops with complex induction variables */
long long test_nested_loops(int N) {
    volatile long long sum = 0;
    
    for (int i = 0; i < N; i++) {
        /* Complex induction variable for inner loop */
        for (int j = i * 2; j < N; j += 3) {
            int k = j + i * 3;
            long long val = (long long)i * j * k;
            
            /* Multiple intermediate computations */
            long long t1 = val * 0x12345678;
            long long t2 = t1 ^ 0xABCDEF;
            asm volatile("" : : "r"(t2));
            
            long long t3 = external_func4(t1);
            
            long long t4 = t2 + t3 * (j + 1);
            asm volatile("" : : "r"(t4));
            
            /* Function call inside inner loop increases pressure */
            if (j % 5 == 0) {
                t4 = external_func4(t4);
            }
            
            sum += t4;
        }
        
        /* Additional computation between outer loop iterations */
        int temp = external_func1(i);
        sum += temp;
        asm volatile("" : : "r"(sum));
    }
    
    return sum;
}

/* Test 4: Vector operations to engage vector modes */
v4si test_vector_operations(int seed) {
    v4si v1 = {seed, seed + 1, seed + 2, seed + 3};
    v4si v2 = {seed * 2, seed * 3, seed * 4, seed * 5};
    v4si v3 = {0x1234, 0x5678, 0x9ABC, 0xDEF0};
    
    /* Multiple vector operations */
    v4si r1 = v1 + v2;
    v4si r2 = v1 * v2;
    v4si r3 = r1 & v3;
    v4si r4 = r2 | v3;
    
    /* Use inline asm to pin vector values */
    asm volatile("" : : "x"(r1), "x"(r2), "x"(r3), "x"(r4));
    
    /* Scalar operations mixed with vector */
    int s1 = seed * 0x1111;
    int s2 = external_func1(s1);
    
    v4si r5 = r3 + (v4si){s1, s2, s1 ^ s2, s1 & s2};
    v4si r6 = r4 * (v4si){s2, s1, s2 ^ 0xFF, s1 | 0xAA};
    
    asm volatile("" : : "x"(r5), "x"(r6));
    
    /* More computations to increase pressure */
    for (int i = 0; i < 4; i++) {
        r5 = r5 + r6;
        r6 = r6 * (v4si){i, i+1, i+2, i+3};
        asm volatile("" : : "x"(r5), "x"(r6));
    }
    
    return r5 + r6;
}

/* Test 5: Extreme register pressure with all types */
double test_extreme_pressure(int iterations) {
    volatile double final_result = 0.0;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Integer computations */
        int i1 = iter * 3;
        int i2 = iter + 0x1234;
        int i3 = i1 ^ i2;
        int i4 = external_func1(i3);
        
        /* Floating computations */
        double f1 = (double)iter * 1.234;
        double f2 = f1 / 3.14159;
        double f3 = external_func3(f2);
        
        /* Long long computations */
        long long ll1 = (long long)iter * 0x100000000LL;
        long long ll2 = external_func4(ll1);
        
        /* Vector computations */
        v4si vec1 = {i1, i2, i3, i4};
        v4si vec2 = {(int)f1, (int)f2, (int)f3, iter};
        v4si vec3 = vec1 + vec2;
        
        /* Pin all intermediate values */
        asm volatile("" : : "r"(i1), "r"(i2), "r"(i3), "r"(i4),
                         "f"(f1), "f"(f2), "f"(f3),
                         "r"(ll1), "r"(ll2),
                         "x"(vec1), "x"(vec2), "x"(vec3));
        
        /* Complex expression combining all types */
        double temp = f3 * (double)i4 + (double)(ll2 & 0xFFFFFFFF);
        for (int j = 0; j < 3; j++) {
            temp = temp * 1.1 - (double)j;
            i4 = external_func2(i4, j);
            asm volatile("" : : "f"(temp), "r"(i4));
        }
        
        final_result += temp + (double)vec3[0];
    }
    
    return final_result;
}

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : time(NULL);
    int N = argc > 2 ? atoi(argv[2]) : 100;
    
    printf("Testing early rematerialization coverage...\n");
    
    /* Run all tests to trigger different parts of the pass */
    int r1 = test_integer_pressure(seed);
    printf("Test 1 result: %d\n", r1);
    
    double r2 = test_mixed_pressure((double)seed);
    printf("Test 2 result: %f\n", r2);
    
    long long r3 = test_nested_loops(N);
    printf("Test 3 result: %lld\n", r3);
    
    v4si r4 = test_vector_operations(seed);
    printf("Test 4 result: %d %d %d %d\n", r4[0], r4[1], r4[2], r4[3]);
    
    double r5 = test_extreme_pressure(N / 10);
    printf("Test 5 result: %f\n", r5);
    
    /* Final volatile store to prevent optimization */
    volatile int checksum = r1 + (int)r2 + (int)r3 + r4[0] + (int)r5;
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
