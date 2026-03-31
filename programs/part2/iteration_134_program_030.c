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
int ext_func2(int x, int y) { return x * y + (x ^ y); }
double ext_func3(double x) { return x * 1.23456789; }
long long ext_func4(long long x) { return x * 3 + 1; }

/* Vector type to test different modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High integer register pressure with complex expressions */
int test_integer_pressure(int seed) {
    volatile int result = 0;
    int a = seed * 3 + 1;
    int b = seed / 2 - 5;
    int c = seed ^ 0x12345678;
    int d = seed + 777;
    
    /* Long sequence of independent computations */
    int t1 = a * b + c - d;
    asm volatile("" : : "r"(t1)); /* Pin in register */
    
    int t2 = (a & b) | (c ^ d);
    int t3 = t1 * t2 - a + b;
    asm volatile("" : : "r"(t3));
    
    int t4 = ext_func1(t3); /* Function call creates pressure point */
    
    int t5 = t4 << 3 | t2 >> 2;
    int t6 = t5 * 7 + t1 / 3;
    asm volatile("" : : "r"(t6));
    
    int t7 = ext_func2(t6, t4);
    
    int t8 = (t7 & 0xFF) + (t5 | 0xAA);
    int t9 = t8 * 11 - t3 * 13;
    asm volatile("" : : "r"(t9));
    
    int t10 = t9 ^ t6 ^ t2;
    int t11 = t10 * 17 + t8 / 5;
    
    /* More computations to increase pressure */
    for (int i = 0; i < 8; i++) {
        t11 = t11 * 3 + i;
        asm volatile("" : : "r"(t11));
        t11 = ext_func1(t11);
    }
    
    result = t11;
    return result;
}

/* Test 2: Mixed integer and floating-point pressure */
double test_mixed_pressure(double seed) {
    volatile double result = 0.0;
    double d1 = seed * 1.5;
    double d2 = seed / 3.0;
    int i1 = (int)seed * 7;
    int i2 = (int)(seed * 2.0);
    
    /* Mixed computations */
    double dt1 = d1 * d2 + 3.14159;
    asm volatile("" : : "r"(i1), "r"(i2)); /* Pin integers */
    
    int it1 = i1 * i2 + (int)d1;
    double dt2 = ext_func3(dt1); /* FP function call */
    
    int it2 = ext_func1(it1);
    double dt3 = dt2 * 2.0 - d1;
    asm volatile("" : : "r"(it2));
    
    /* More complex mixed expressions */
    for (int i = 0; i < 5; i++) {
        dt3 = dt3 * 1.1 + (double)i;
        it2 = it2 * 2 + i;
        asm volatile("" : : "r"(it2));
        dt3 = ext_func3(dt3);
    }
    
    result = dt3 + (double)it2;
    return result;
}

/* Test 3: Nested loops with complex induction variables */
long long test_nested_loops(int N) {
    volatile long long sum = 0;
    
    for (int i = 0; i < N; i++) {
        /* Non-trivial induction variable for inner loop */
        for (int j = i * 2; j < N; j += 3) {
            int k = j * 3 - i;
            long long val = (long long)i * j * k;
            
            /* Complex computation in loop body */
            val = val + (i ^ j) * (k & 0xFF);
            asm volatile("" : : "r"(val));
            
            /* Function call increases pressure */
            val = ext_func4(val);
            
            /* More computations */
            for (int m = 0; m < 2; m++) {
                val = val * 2 + m;
                int temp = (int)val & 0xFFFF;
                asm volatile("" : : "r"(temp));
                temp = ext_func1(temp);
                val += temp;
            }
            
            sum += val;
            asm volatile("" : : "r"(sum));
        }
    }
    
    return sum;
}

/* Test 4: Vector operations for vector modes */
v4si test_vector_ops(v4si a, v4si b) {
    v4si result;
    
    /* Vector computations */
    v4si t1 = a + b;
    v4si t2 = a * b;
    v4si t3 = t1 - b;
    
    /* Scalar operations mixed in */
    int s1 = ((int*)&t1)[0] + ((int*)&t2)[1];
    asm volatile("" : : "r"(s1));
    s1 = ext_func1(s1);
    
    v4si t4 = t2 + t3;
    int s2 = ((int*)&t3)[2] * ((int*)&t4)[3];
    asm volatile("" : : "r"(s2));
    
    /* More vector ops */
    for (int i = 0; i < 4; i++) {
        ((int*)&t4)[i] = ((int*)&t4)[i] * 2 + i;
        asm volatile("" : : "r"(((int*)&t4)[i]));
    }
    
    result = t4 + t1;
    return result;
}

/* Test 5: Extreme register pressure with many live values */
int test_extreme_pressure(int iterations) {
    volatile int final = 0;
    
    /* Create many live variables */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    
    /* Force all to be live simultaneously */
    asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4), "r"(v5),
                       "r"(v6), "r"(v7), "r"(v8), "r"(v9), "r"(v10),
                       "r"(v11), "r"(v12), "r"(v13), "r"(v14), "r"(v15));
    
    for (int i = 0; i < iterations; i++) {
        /* Complex expressions using all variables */
        v1 = v1 * v2 + v3;
        v2 = v2 ^ v4 | v5;
        v3 = v3 + v6 - v7;
        v4 = v4 * v8 / (v9 + 1);
        v5 = v5 & v10 ^ v11;
        
        /* Function calls create pressure points */
        v6 = ext_func1(v6);
        v7 = ext_func2(v7, v8);
        
        v8 = v8 + v12 * v13;
        v9 = v9 | v14 & v15;
        v10 = v10 * 3 + v1;
        
        /* Pin intermediate values */
        asm volatile("" : : "r"(v1), "r"(v2), "r"(v3));
        
        v11 = ext_func1(v11);
        v12 = v12 + v4 - v5;
        v13 = v13 * 7 ^ v6;
        v14 = v14 & 0xFF | v7;
        v15 = v15 * 11 + v8;
        
        final += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15;
    }
    
    return final;
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
    
    /* Vector test */
    v4si va = {1, 2, 3, 4};
    v4si vb = {5, 6, 7, 8};
    v4si r4 = test_vector_ops(va, vb);
    printf("Test 4 result: [%d, %d, %d, %d]\n", 
           ((int*)&r4)[0], ((int*)&r4)[1], ((int*)&r4)[2], ((int*)&r4)[3]);
    
    int r5 = test_extreme_pressure(5);
    printf("Test 5 result: %d\n", r5);
    
    /* Final checksum to prevent optimization */
    volatile int checksum = r1 + (int)r2 + (int)r3 + 
                           ((int*)&r4)[0] + r5;
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
