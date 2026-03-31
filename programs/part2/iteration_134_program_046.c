/* test_early_remat.c - Test program for GCC early rematerialization pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External non-inline functions to force register pressure */
extern int ext_func1(int);
extern int ext_func2(int, int);
extern double ext_func3(double);
extern long long ext_func4(long long);

/* Prevent optimization */
#define KEEP(var) asm volatile("" : : "r"(var))
#define KEEP_MEM(var) asm volatile("" : : "m"(var))

/* Vector type to engage different modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High integer register pressure with complex expressions */
int test_high_int_pressure(int seed) {
    int a = seed * 3;
    int b = seed + 7;
    int c = seed ^ 0x55AA55AA;
    int d = seed << 3;
    
    /* Long sequence of independent computations */
    int t1 = a * b + c;
    KEEP(t1);
    int t2 = (b & c) | (d ^ a);
    KEEP(t2);
    int t3 = t1 * t2 - seed;
    KEEP(t3);
    int t4 = ext_func1(t3);  /* Function call creates pressure point */
    int t5 = t4 * 7 + t2;
    KEEP(t5);
    int t6 = (t5 << 2) | (t3 >> 1);
    KEEP(t6);
    int t7 = t6 ^ t4 ^ t1;
    KEEP(t7);
    int t8 = ext_func2(t7, t6);  /* Another pressure point */
    int t9 = t8 * 3 + t5 * 2;
    KEEP(t9);
    int t10 = (t9 & 0xFF) | (t8 << 8);
    KEEP(t10);
    
    /* More computations to increase pressure */
    for (int i = 0; i < 4; i++) {
        t10 = t10 * 2 + i;
        KEEP(t10);
        t9 = t9 ^ (t10 << i);
        KEEP(t9);
    }
    
    return t9 + t10;
}

/* Test 2: Mixed integer and floating-point pressure */
double test_mixed_pressure(int seed) {
    double f1 = seed * 1.5;
    double f2 = seed / 3.14159;
    int i1 = seed * 2;
    int i2 = seed + 1000;
    
    KEEP(f1);
    KEEP(f2);
    
    /* Interleave FP and integer ops */
    double f3 = f1 * f2 + ext_func3(f1);
    int i3 = i1 * i2 - ext_func1(i1);
    
    KEEP(f3);
    KEEP(i3);
    
    /* More mixed computations */
    for (int i = 0; i < 3; i++) {
        f3 = f3 * 1.1 + i;
        i3 = i3 ^ (i * 0x1234);
        KEEP(f3);
        KEEP(i3);
        
        /* Function call that uses both types */
        f3 = ext_func3(f3);
        i3 = ext_func1(i3);
    }
    
    return f3 + i3;
}

/* Test 3: Nested loops with complex induction */
long long test_nested_loops(int N) {
    long long sum = 0;
    
    for (int i = 0; i < N; i++) {
        /* Complex induction variable */
        for (int j = i * 2; j < N; j += 3) {
            /* Inner loop with register pressure */
            int a = i * j;
            int b = j * j - i;
            int c = a ^ b;
            long long d = (long long)a * b;
            
            KEEP(a);
            KEEP(b);
            KEEP(c);
            KEEP(d);
            
            /* Function call in inner loop */
            c = ext_func1(c);
            d = ext_func4(d);
            
            /* More computations */
            for (int k = 0; k < 2; k++) {
                a = a + k * 7;
                b = b ^ (k * 0xABCD);
                c = c * 2 - k;
                d = d + a * b;
                
                KEEP(a);
                KEEP(b);
                KEEP(c);
                KEEP(d);
            }
            
            sum += d + c;
        }
        
        /* External call between loop iterations */
        sum = ext_func4(sum);
    }
    
    return sum;
}

/* Test 4: Vector operations to engage vector modes */
v4si test_vector_ops(int seed) {
    v4si v1 = {seed, seed + 1, seed + 2, seed + 3};
    v4si v2 = {seed * 2, seed * 3, seed * 4, seed * 5};
    v4si v3 = {0};
    
    KEEP_MEM(v1);
    KEEP_MEM(v2);
    
    /* Vector operations */
    v3 = v1 + v2;
    KEEP_MEM(v3);
    
    v3 = v3 * v1;
    KEEP_MEM(v3);
    
    /* Scalar operations mixed in */
    int s1 = seed * 7;
    int s2 = seed ^ 0xF0F0F0F0;
    
    for (int i = 0; i < 4; i++) {
        s1 = s1 + v3[i];
        s2 = s2 ^ v1[i];
        KEEP(s1);
        KEEP(s2);
        
        /* Function calls */
        s1 = ext_func1(s1);
        s2 = ext_func2(s2, i);
        
        v3[i] = v3[i] + s1;
    }
    
    KEEP_MEM(v3);
    
    return v3;
}

/* Test 5: Extreme register pressure with many live values */
int test_extreme_pressure(int iterations) {
    /* Declare many variables to saturate registers */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    
    /* Keep all alive */
    KEEP(v1); KEEP(v2); KEEP(v3); KEEP(v4); KEEP(v5);
    KEEP(v6); KEEP(v7); KEEP(v8); KEEP(v9); KEEP(v10);
    KEEP(v11); KEEP(v12); KEEP(v13); KEEP(v14); KEEP(v15);
    
    for (int i = 0; i < iterations; i++) {
        /* Complex expressions using all variables */
        v1 = v1 * v2 + v3;
        v2 = v2 ^ v4 | v5;
        v3 = v3 + v6 - v7;
        v4 = v4 * v8 / (v9 + 1);
        v5 = v5 & v10 ^ v11;
        v6 = v6 + v12 * v13;
        v7 = v7 | v14 & v15;
        v8 = v8 ^ v1 + v2;
        v9 = v9 * v3 - v4;
        v10 = v10 & v5 | v6;
        v11 = v11 + v7 * v8;
        v12 = v12 ^ v9 & v10;
        v13 = v13 + v11 - v12;
        v14 = v14 * v13 / (v14 + 1);
        v15 = v15 ^ v1 & v2 | v3;
        
        /* Keep all updated values alive */
        KEEP(v1); KEEP(v2); KEEP(v3); KEEP(v4); KEEP(v5);
        KEEP(v6); KEEP(v7); KEEP(v8); KEEP(v9); KEEP(v10);
        KEEP(v11); KEEP(v12); KEEP(v13); KEEP(v14); KEEP(v15);
        
        /* Function call that clobbers registers */
        v1 = ext_func1(v1);
        v2 = ext_func2(v2, v3);
    }
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15;
}

/* Dummy external functions */
int ext_func1(int x) { return x ^ 0x12345678; }
int ext_func2(int x, int y) { return x * y + 1; }
double ext_func3(double x) { return x * 1.2345; }
long long ext_func4(long long x) { return x + 0x1122334455667788LL; }

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : time(NULL);
    int N = argc > 2 ? atoi(argv[2]) : 10;
    int iterations = argc > 3 ? atoi(argv[3]) : 5;
    
    printf("Seed: %d\n", seed);
    
    /* Run all tests to trigger different aspects of rematerialization */
    int result1 = test_high_int_pressure(seed);
    printf("Test 1 result: %d\n", result1);
    
    double result2 = test_mixed_pressure(seed);
    printf("Test 2 result: %f\n", result2);
    
    long long result3 = test_nested_loops(N);
    printf("Test 3 result: %lld\n", result3);
    
    v4si result4 = test_vector_ops(seed);
    printf("Test 4 result: %d %d %d %d\n", 
           result4[0], result4[1], result4[2], result4[3]);
    
    int result5 = test_extreme_pressure(iterations);
    printf("Test 5 result: %d\n", result5);
    
    /* Final checksum to prevent optimization */
    int final = result1 + (int)result2 + (int)result3 + 
                result4[0] + result5;
    printf("Final checksum: %d\n", final);
    
    return final != 0 ? 0 : 1;
}
