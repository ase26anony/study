/* test_early_remat.c - Test program for GCC early rematerialization pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External non-inline functions to create register pressure points */
extern int external_func1(int);
extern int external_func2(int, int);
extern double external_func3(double);
extern long long external_func4(long long);

/* Dummy implementations to avoid linking errors */
int external_func1(int x) { return x ^ 0x55AA55AA; }
int external_func2(int x, int y) { return (x * y) ^ 0x12345678; }
double external_func3(double x) { return x * 3.141592653589793; }
long long external_func4(long long x) { return x * 0xDEADBEEFCAFEBABEULL; }

/* Vector type to test different modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High register pressure with mixed integer operations */
int test_mixed_integer_pressure(int seed) {
    volatile int result = 0;
    int a = seed * 3 + 1;
    int b = seed / 2 - 5;
    int c = seed ^ 0x1234;
    int d = seed + 777;
    
    /* Force values into registers */
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
    
    /* Long sequence of independent computations */
    int t1 = a * b + c - d;
    int t2 = (a & b) | (c ^ d);
    int t3 = t1 * t2 + seed;
    int t4 = t2 - t1 * 3;
    int t5 = (t3 << 4) | (t4 >> 2);
    int t6 = t5 ^ t4 ^ t3;
    
    asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4), "r"(t5), "r"(t6));
    
    /* Function call creates register pressure */
    int t7 = external_func1(t6);
    
    /* More computations */
    int t8 = t7 * 17 + t6;
    int t9 = t8 & 0xFF00FF;
    int t10 = t9 | (t7 << 8);
    int t11 = t10 - t9 + t8;
    int t12 = t11 * t10 / (t9 + 1);
    
    asm volatile("" : : "r"(t7), "r"(t8), "r"(t9), "r"(t10), "r"(t11), "r"(t12));
    
    /* Another function call */
    int t13 = external_func2(t12, t11);
    
    /* Final computation chain */
    for (int i = 0; i < 8; i++) {
        t13 = t13 * 3 + i;
        asm volatile("" : : "r"(t13));
    }
    
    result = t13;
    return result;
}

/* Test 2: Mixed floating-point and integer operations */
double test_mixed_fp_int_pressure(double seed) {
    volatile double result = 0.0;
    double f1 = seed * 1.234567;
    double f2 = seed / 0.987654;
    int i1 = (int)(seed * 1000);
    int i2 = (int)(seed * 2000);
    
    asm volatile("" : : "r"(f1), "r"(f2), "r"(i1), "r"(i2));
    
    /* Mixed computations */
    double f3 = f1 * f2 + (double)i1;
    int i3 = i1 * i2 + (int)f1;
    double f4 = f3 / (f2 + 1.0);
    int i4 = i3 ^ (int)f4;
    
    asm volatile("" : : "r"(f3), "r"(i3), "r"(f4), "r"(i4));
    
    /* Function call with floating point */
    double f5 = external_func3(f4);
    
    /* More mixed ops */
    double f6 = f5 * (double)i4 + f3;
    int i5 = i4 * (int)f5 + i3;
    double f7 = f6 - f5 * 2.5;
    int i6 = (i5 << 3) | (int)f7;
    
    asm volatile("" : : "r"(f5), "r"(f6), "r"(i5), "r"(f7), "r"(i6));
    
    /* Nested loops with derived induction variables */
    double acc = 0.0;
    for (int i = 0; i < 100; i++) {
        for (int j = i * 2; j < i * 2 + 10; j += 3) {
            acc += f7 * j + f6 * i + (double)(i6 ^ j);
            asm volatile("" : : "r"(acc));
        }
        /* Function call in loop */
        i6 = external_func1(i6);
    }
    
    result = acc + f7;
    return result;
}

/* Test 3: 64-bit and vector operations */
long long test_64bit_vector_pressure(long long seed) {
    volatile long long result = 0;
    long long ll1 = seed * 0x123456789ABCDEFULL;
    long long ll2 = seed ^ 0xFEDCBA9876543210ULL;
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    asm volatile("" : : "r"(ll1), "r"(ll2));
    
    /* 64-bit computations */
    long long ll3 = ll1 * ll2 + seed;
    long long ll4 = (ll1 & ll2) | (seed << 32);
    long long ll5 = ll3 ^ ll4 ^ 0xAAAAAAAAAAAAAAAALL;
    
    asm volatile("" : : "r"(ll3), "r"(ll4), "r"(ll5));
    
    /* Vector operations */
    v4si vec3 = vec1 + vec2;
    v4si vec4 = vec1 * vec2;
    v4si vec5 = vec3 & vec4;
    
    /* Function call with 64-bit */
    long long ll6 = external_func4(ll5);
    
    /* Mixed 64-bit and vector */
    long long ll7 = ll6 * 31 + (long long)vec5[0];
    v4si vec6 = vec5 * (v4si){ll7 & 0xFF, (ll7 >> 8) & 0xFF, 
                              (ll7 >> 16) & 0xFF, (ll7 >> 24) & 0xFF};
    
    asm volatile("" : : "r"(ll6), "r"(ll7));
    
    /* Complex loop with multiple induction variables */
    long long sum = 0;
    for (long long i = 0; i < 50; i++) {
        for (long long j = i * 3; j < i * 3 + 20; j += 2) {
            for (long long k = j * 2; k < j * 2 + 5; k++) {
                sum += ll7 * k + ll6 * j + vec6[k % 4];
                asm volatile("" : : "r"(sum));
            }
            /* Function call in inner loop */
            ll7 = external_func4(ll7 ^ j);
        }
        vec6 = vec6 + (v4si){i, i+1, i+2, i+3};
    }
    
    result = sum + ll7;
    return result;
}

/* Test 4: Extreme register pressure with many live values */
int test_extreme_pressure(int seed) {
    volatile int result = 0;
    
    /* Create many live variables */
    int v[20];
    for (int i = 0; i < 20; i++) {
        v[i] = seed * i + i * i;
        asm volatile("" : : "r"(v[i]));
    }
    
    /* Long dependency chain with many intermediates */
    int x = v[0];
    for (int i = 1; i < 20; i++) {
        int y = x * v[i] + v[i-1];
        int z = y ^ v[(i+1) % 20];
        int w = z * 3 - y / 2;
        x = w + v[i % 5];
        
        /* Function call every few iterations */
        if (i % 3 == 0) {
            x = external_func1(x);
        }
        
        /* Force all intermediates to be live */
        asm volatile("" : : "r"(y), "r"(z), "r"(w), "r"(x));
    }
    
    /* Additional computation with all values */
    int final = 0;
    for (int i = 0; i < 20; i++) {
        final += v[i] * (i + 1);
        final ^= x;
        x = x * 1103515245 + 12345;
        asm volatile("" : : "r"(final), "r"(x));
    }
    
    result = final;
    return result;
}

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : time(NULL);
    
    printf("Testing early rematerialization coverage...\n");
    
    /* Run all tests to exercise different patterns */
    int r1 = test_mixed_integer_pressure(seed);
    printf("Test 1 result: %d\n", r1);
    
    double r2 = test_mixed_fp_int_pressure((double)seed / 1000.0);
    printf("Test 2 result: %f\n", r2);
    
    long long r3 = test_64bit_vector_pressure(seed * 1000LL);
    printf("Test 3 result: %lld\n", r3);
    
    int r4 = test_extreme_pressure(seed ^ 0x55AA55AA);
    printf("Test 4 result: %d\n", r4);
    
    /* Final checksum to prevent optimization */
    volatile int checksum = r1 + (int)r2 + (int)r3 + r4;
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
