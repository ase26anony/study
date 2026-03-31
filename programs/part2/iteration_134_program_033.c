/* test_early_remat.c - Test program for GCC early rematerialization pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External non-inline functions to force register pressure */
extern int external_func1(int);
extern int external_func2(int, int);
extern double external_func3(double);
extern long long external_func4(long long);

/* Prevent optimization */
static volatile int global_counter = 0;

/* Vector type to engage different modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High integer register pressure with complex expressions */
int test_high_int_pressure(int seed) {
    int a = seed * 3;
    int b = seed + 7;
    int c = seed ^ 0x55AA55AA;
    int d = seed << 3;
    int e = seed >> 2;
    int f = seed | 0x12345678;
    int g = seed & 0xF0F0F0F0;
    int h = seed % 31;
    
    /* Force values into registers */
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
    
    /* Complex chain of independent computations */
    int t1 = a * b + c - d;
    int t2 = (e & f) | (g ^ h);
    int t3 = t1 * t2 + (a << 2);
    int t4 = (b >> 1) ^ (c & d);
    int t5 = t3 - t4 * e;
    int t6 = f + g * h - t5;
    
    asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4));
    
    /* Function call creates pressure point */
    int r1 = external_func1(t5);
    
    int t7 = t6 * r1 + seed;
    int t8 = (t7 << 3) ^ (r1 >> 2);
    int t9 = t8 - external_func2(t7, t8);
    int t10 = t9 | (t7 & t8);
    
    asm volatile("" : : "r"(t5), "r"(t6), "r"(t7), "r"(t8));
    
    /* More independent computations */
    for (int i = 0; i < 4; i++) {
        t10 += external_func1(t10 + i);
        asm volatile("" : : "r"(t10));
    }
    
    return t10;
}

/* Test 2: Mixed integer and floating-point operations */
double test_mixed_types(int seed) {
    double d1 = seed * 1.5;
    double d2 = seed / 3.14159;
    float f1 = seed * 0.25f;
    float f2 = seed + 0.75f;
    
    int i1 = seed * 3;
    int i2 = seed + 100;
    long long ll1 = (long long)seed * 1000000LL;
    long long ll2 = (long long)seed << 8;
    
    asm volatile("" : : "r"(i1), "r"(i2), "r"(ll1), "r"(ll2));
    
    /* Mixed computations */
    double t1 = d1 * d2 + f1 - f2;
    int t2 = i1 * i2 + (int)d1;
    long long t3 = ll1 + ll2 * (long long)t2;
    float t4 = f1 * f2 + (float)t1;
    
    asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4));
    
    /* Function calls with different types */
    double r1 = external_func3(t1);
    long long r2 = external_func4(t3);
    
    /* More mixed operations */
    double result = r1 * t4 + (double)r2 / 1000.0;
    result += external_func3(result);
    
    for (int i = 0; i < 3; i++) {
        result += external_func1((int)result + i) * 0.1;
        asm volatile("" : : "r"(result));
    }
    
    return result;
}

/* Test 3: Nested loops with complex induction */
int test_nested_loops(int N) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        /* Non-trivial induction variable */
        for (int j = i * 2; j < N; j += 3) {
            /* Complex expression using both i and j */
            int val = (i * j) + (i << 3) - (j >> 1);
            
            /* Additional computations to increase pressure */
            int t1 = val ^ 0xAAAAAAAA;
            int t2 = t1 * 3 + j;
            int t3 = (t2 & 0x55555555) | (i << 16);
            int t4 = t3 - external_func1(t2);
            
            asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4));
            
            sum += t4;
            
            /* Periodic function call */
            if ((j & 7) == 0) {
                sum += external_func2(i, j);
            }
        }
        
        /* More computations between loop iterations */
        int outer_temp = i * 7 + sum;
        asm volatile("" : : "r"(outer_temp));
        sum += external_func1(outer_temp);
    }
    
    return sum;
}

/* Test 4: Vector operations */
int test_vector_ops(int seed) {
    v4si v1 = {seed, seed + 1, seed + 2, seed + 3};
    v4si v2 = {seed * 2, seed * 3, seed * 4, seed * 5};
    v4si v3 = {seed ^ 1, seed ^ 2, seed ^ 3, seed ^ 4};
    
    /* Vector operations */
    v4si r1 = v1 + v2;
    v4si r2 = v1 * v3;
    v4si r3 = r1 & v2;
    v4si r4 = r2 | r3;
    
    /* Extract and use individual elements */
    int arr[4];
    for (int i = 0; i < 4; i++) {
        arr[i] = r4[i];
    }
    
    asm volatile("" : : "r"(arr[0]), "r"(arr[1]), "r"(arr[2]), "r"(arr[3]));
    
    /* Mix with scalar operations */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        int temp = arr[i] * 3 + external_func1(arr[i]);
        sum += temp;
        asm volatile("" : : "r"(temp));
    }
    
    /* Additional vector-scalar mixing */
    v4si v4 = {sum, sum + 1, sum + 2, sum + 3};
    v4si r5 = v4 * r1;
    
    for (int i = 0; i < 4; i++) {
        sum += r5[i] + external_func2(i, r5[i]);
    }
    
    return sum;
}

/* Test 5: Extreme register pressure with many live values */
long long test_extreme_pressure(int iterations) {
    long long accum = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Create many independent live values */
        long long v1 = accum + iter;
        long long v2 = v1 * 3;
        long long v3 = v2 ^ 0x123456789ABCDEF0LL;
        long long v4 = v3 << 2;
        long long v5 = v4 >> 1;
        long long v6 = v5 | 0xF0F0F0F0F0F0F0F0LL;
        long long v7 = v6 & 0x0F0F0F0F0F0F0F0FLL;
        long long v8 = v7 + v1;
        long long v9 = v8 * v2;
        long long v10 = v9 - v3;
        
        /* Force all into registers */
        asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4), 
                               "r"(v5), "r"(v6), "r"(v7), "r"(v8));
        
        /* Complex expression using many values */
        long long result = (v1 * v2) + (v3 & v4) - (v5 | v6) ^ (v7 + v8) * (v9 - v10);
        
        /* Function call that forces spill/remat decisions */
        result += external_func4(result);
        
        /* More computations keeping values live */
        v10 += external_func1((int)v10);
        v9 ^= external_func2((int)v9, (int)v10);
        
        asm volatile("" : : "r"(v9), "r"(v10), "r"(result));
        
        accum += result;
        
        /* Reset some values to create varying live ranges */
        if ((iter & 3) == 0) {
            v1 = external_func4(accum);
            v2 = v1 * 2;
            asm volatile("" : : "r"(v1), "r"(v2));
        }
    }
    
    return accum;
}

/* Dummy external functions */
int external_func1(int x) { return x ^ 0x1234; }
int external_func2(int x, int y) { return x * y + 0x5678; }
double external_func3(double x) { return x * 1.2345; }
long long external_func4(long long x) { return x + 0x9ABCDEF012345678LL; }

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : time(NULL);
    int N = argc > 2 ? atoi(argv[2]) : 100;
    
    printf("Testing early rematerialization coverage...\n");
    
    /* Run all tests to exercise different patterns */
    int r1 = test_high_int_pressure(seed);
    printf("Test 1 result: %d\n", r1);
    
    double r2 = test_mixed_types(seed);
    printf("Test 2 result: %f\n", r2);
    
    int r3 = test_nested_loops(N);
    printf("Test 3 result: %d\n", r3);
    
    int r4 = test_vector_ops(seed);
    printf("Test 4 result: %d\n", r4);
    
    long long r5 = test_extreme_pressure(N / 10);
    printf("Test 5 result: %lld\n", r5);
    
    /* Use results to prevent optimization */
    global_counter = r1 + (int)r2 + r3 + r4 + (int)r5;
    
    printf("Final checksum: %d\n", global_counter);
    
    return 0;
}
