/* test_early_remat.c - Test program for GCC early rematerialization pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External non-inline functions to force register pressure */
extern int ext_func1(int);
extern double ext_func2(double);
extern long long ext_func3(long long);

/* Vector type to engage different modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High register pressure with mixed integer operations */
int test_high_pressure_int(int seed) {
    volatile int result = 0;
    int a = seed * 3;
    int b = seed + 7;
    int c = seed - 2;
    int d = seed ^ 0x55AA55AA;
    int e = seed * 13;
    int f = seed / 5;
    
    /* Force values into registers */
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f));
    
    /* Long sequence of independent computations */
    int t1 = a * b + c;
    int t2 = d & e | f;
    int t3 = t1 ^ t2;
    int t4 = t3 * 17 - 23;
    int t5 = t4 >> 3;
    int t6 = t5 * t1;
    int t7 = t6 + t2 * 3;
    int t8 = t7 & 0xFF;
    int t9 = t8 * t3;
    int t10 = t9 - t4;
    
    /* Function call to clobber caller-saved registers */
    int r1 = ext_func1(t10);
    
    /* More computations after call */
    int t11 = r1 * t5;
    int t12 = t11 + t6;
    int t13 = t12 ^ t7;
    int t14 = t13 * 19;
    int t15 = t14 >> 1;
    int t16 = t15 & t8;
    int t17 = t16 * 7;
    int t18 = t17 - t9;
    int t19 = t18 + t10;
    int t20 = t19 ^ r1;
    
    /* Another function call */
    int r2 = ext_func1(t20);
    
    /* Final computation chain */
    int t21 = t11 * t12 + t13;
    int t22 = t14 & t15 | t16;
    int t23 = t17 ^ t18 * t19;
    int t24 = t20 + t21 - t22;
    int t25 = t23 * t24;
    
    result = t25 + r2;
    asm volatile("" : : "r"(result));
    return result;
}

/* Test 2: Mixed floating-point and integer operations */
double test_mixed_fp_int(double seed) {
    volatile double result = 0.0;
    double d1 = seed * 1.5;
    double d2 = seed + 2.71828;
    double d3 = seed - 3.14159;
    int i1 = (int)seed * 7;
    int i2 = (int)(seed * 11);
    
    asm volatile("" : : "r"(d1), "r"(d2), "r"(d3), "r"(i1), "r"(i2));
    
    /* Mixed computations */
    double t1 = d1 * d2 + d3;
    int t2 = i1 & i2 | (int)t1;
    double t3 = t1 * 2.5 - (double)t2;
    int t4 = t2 * 3 + (int)(t3 * 4.0);
    
    /* Function call with double */
    double r1 = ext_func2(t3);
    
    /* More mixed ops */
    double t5 = r1 * d1;
    int t6 = t4 ^ (int)t5;
    double t7 = t5 + (double)t6 / 256.0;
    int t8 = t6 * 5 - (int)(t7 * 100.0);
    
    /* Another call */
    double r2 = ext_func2(t7);
    
    double t9 = t7 * r2;
    int t10 = t8 & 0xFF;
    double t11 = t9 + (double)t10;
    
    result = t11;
    asm volatile("" : : "r"(result));
    return result;
}

/* Test 3: Nested loops with complex induction */
long long test_nested_loops(int N) {
    volatile long long sum = 0;
    
    for (int i = 0; i < N; i++) {
        /* Complex induction variable */
        for (int j = i * 2; j < N; j += 3) {
            /* Inner loop with register pressure */
            int a = i * j + 7;
            int b = j * 13 - i;
            int c = a ^ b;
            int d = c * 17;
            
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
            
            /* Function call in inner loop */
            long long r1 = ext_func3((long long)d);
            
            int e = d + (int)(r1 & 0xFFFF);
            int f = e * 3;
            int g = f ^ a;
            
            sum += g + r1;
            
            /* More computations to increase pressure */
            int h = g * 5;
            int k = h - b;
            int l = k & 0xFF;
            int m = l * 7;
            
            asm volatile("" : : "r"(h), "r"(k), "r"(l), "r"(m));
        }
        
        /* Additional computation between outer loop iterations */
        int x = i * 19;
        int y = x ^ 0x12345678;
        int z = y * 31;
        
        asm volatile("" : : "r"(x), "r"(y), "r"(z));
        
        sum += z;
    }
    
    asm volatile("" : : "r"(sum));
    return sum;
}

/* Test 4: Vector operations to engage vector modes */
v4si test_vector_ops(v4si v1, v4si v2) {
    volatile v4si result;
    
    /* Multiple vector operations */
    v4si t1 = v1 + v2;
    v4si t2 = v1 * v2;
    v4si t3 = t1 & t2;
    v4si t4 = t1 | t2;
    v4si t5 = t3 ^ t4;
    
    /* Scalar extractions to force different modes */
    int s1 = t1[0] + t2[1];
    int s2 = t3[2] * t4[3];
    int s3 = s1 ^ s2;
    
    asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4), "r"(t5), "r"(s1), "r"(s2), "r"(s3));
    
    /* More vector operations */
    v4si t6 = t5 * 3;
    v4si t7 = t6 + v1;
    v4si t8 = t7 & 0xFF;
    
    /* Function call with scalar */
    int r1 = ext_func1(s3);
    
    v4si t9 = t8 * r1;
    result = t9 + t6;
    
    asm volatile("" : : "r"(result));
    return result;
}

/* Test 5: Extreme register pressure with many temporaries */
long long test_extreme_pressure(int iterations) {
    volatile long long checksum = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Create many unique temporary values */
        long long v1 = iter * 3LL;
        long long v2 = iter + 7LL;
        long long v3 = iter ^ 0xF0F0F0F0F0F0F0F0LL;
        long long v4 = v1 * v2;
        long long v5 = v3 + v4;
        long long v6 = v5 & 0xFFFFFFFFLL;
        long long v7 = v6 * 13LL;
        long long v8 = v7 - v1;
        long long v9 = v8 ^ v2;
        long long v10 = v9 * 17LL;
        long long v11 = v10 >> 3;
        long long v12 = v11 + v3;
        long long v13 = v12 & v4;
        long long v14 = v13 * 19LL;
        long long v15 = v14 - v5;
        long long v16 = v15 ^ v6;
        long long v17 = v16 * 23LL;
        long long v18 = v17 >> 2;
        long long v19 = v18 + v7;
        long long v20 = v19 & v8;
        
        /* Force all values to be live */
        asm volatile("" : : 
            "r"(v1), "r"(v2), "r"(v3), "r"(v4), "r"(v5),
            "r"(v6), "r"(v7), "r"(v8), "r"(v9), "r"(v10),
            "r"(v11), "r"(v12), "r"(v13), "r"(v14), "r"(v15),
            "r"(v16), "r"(v17), "r"(v18), "r"(v19), "r"(v20));
        
        /* Function call to force spill/remat decisions */
        long long r1 = ext_func3(v20);
        
        /* Continue with more computations */
        long long v21 = v20 * r1;
        long long v22 = v21 + v1 - v2;
        long long v23 = v22 ^ v3;
        long long v24 = v23 * 29LL;
        long long v25 = v24 & v4;
        
        checksum += v25;
    }
    
    asm volatile("" : : "r"(checksum));
    return checksum;
}

/* External function implementations */
int ext_func1(int x) {
    return x ^ 0x12345678;
}

double ext_func2(double x) {
    return x * 1.6180339887;
}

long long ext_func3(long long x) {
    return x + 0x1122334455667788LL;
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    int N = argc > 2 ? atoi(argv[2]) : 100;
    
    printf("Testing early rematerialization coverage...\n");
    
    /* Run all tests to exercise different patterns */
    int r1 = test_high_pressure_int(seed);
    printf("Test 1 result: %d\n", r1);
    
    double r2 = test_mixed_fp_int((double)seed);
    printf("Test 2 result: %f\n", r2);
    
    long long r3 = test_nested_loops(N);
    printf("Test 3 result: %lld\n", r3);
    
    v4si v1 = {seed, seed+1, seed+2, seed+3};
    v4si v2 = {seed*2, seed*3, seed*4, seed*5};
    v4si r4 = test_vector_ops(v1, v2);
    printf("Test 4 result: {%d, %d, %d, %d}\n", r4[0], r4[1], r4[2], r4[3]);
    
    long long r5 = test_extreme_pressure(N/10);
    printf("Test 5 result: %lld\n", r5);
    
    /* Final checksum to prevent optimization */
    volatile long long final = r1 + (long long)r2 + r3 + r4[0] + r5;
    printf("Final checksum: %lld\n", final);
    
    return 0;
}
