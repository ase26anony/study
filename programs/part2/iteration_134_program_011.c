/* test_early_remat.c - Test program for GCC early rematerialization pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External non-inline functions to create register pressure points */
extern int external_func1(int);
extern int external_func2(int, int);
extern double external_func3(double);
extern long long external_func4(long long);

/* Dummy external functions to prevent inlining */
int external_func1(int x) { return x ^ 0x55AA55AA; }
int external_func2(int x, int y) { return (x * y) ^ 0x12345678; }
double external_func3(double x) { return x * 3.141592653589793; }
long long external_func4(long long x) { return x * 6364136223846793005LL; }

/* Vector type to engage different modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High integer register pressure with complex expressions */
int test_integer_pressure(int seed) {
    volatile int result = 0;
    int a = seed * 3 + 1;
    int b = seed / 2 - 5;
    
    /* Long sequence of independent computations */
    int t1 = a * b + seed;
    asm volatile("" : : "r"(t1)); /* Force t1 to be in register */
    
    int t2 = (a & 0xFF) | (b << 8);
    int t3 = t1 ^ t2 ^ 0xDEADBEEF;
    asm volatile("" : : "r"(t3));
    
    int t4 = external_func1(t3); /* Function call creates pressure */
    
    int t5 = t4 * 7 - 13;
    int t6 = (t5 >> 4) & 0x0F0F0F0F;
    asm volatile("" : : "r"(t6));
    
    int t7 = external_func2(t6, t4);
    
    int t8 = t7 + t5 - t3;
    int t9 = (t8 * 3) / 2;
    asm volatile("" : : "r"(t9));
    
    int t10 = t9 ^ t8 ^ t7 ^ t6;
    int t11 = external_func1(t10);
    
    int t12 = (t11 << 1) | (t11 >> 31);
    int t13 = t12 + a + b;
    asm volatile("" : : "r"(t13));
    
    int t14 = external_func2(t13, t12);
    
    int t15 = t14 * 0x9E3779B9;
    int t16 = (t15 + t13) ^ t11;
    asm volatile("" : : "r"(t16));
    
    result = t16;
    return result;
}

/* Test 2: Mixed integer and floating-point operations */
double test_mixed_types(int seed) {
    volatile double result = 0.0;
    double d1 = seed * 1.5;
    double d2 = seed / 3.14159;
    
    /* Mix float and int operations */
    int i1 = (int)d1 * 3;
    double d3 = d1 * d2 + i1;
    asm volatile("" : : "r"(i1), "r"(d3));
    
    double d4 = external_func3(d3);
    
    int i2 = (int)d4 ^ 0x55AA55AA;
    double d5 = d4 * 2.71828 - i2;
    asm volatile("" : : "r"(i2), "r"(d5));
    
    int i3 = external_func1(i2);
    double d6 = external_func3(d5);
    
    double d7 = d6 + d5 * i3;
    int i4 = (int)d7 & 0x7FFFFFFF;
    asm volatile("" : : "r"(i4), "r"(d7));
    
    double d8 = d7 / (i4 + 1.0);
    result = d8;
    return result;
}

/* Test 3: Nested loops with complex induction variables */
long long test_nested_loops(int N) {
    volatile long long total = 0;
    
    for (int i = 0; i < N; i++) {
        /* Non-trivial induction variable for inner loop */
        for (int j = i * 2; j < N; j += 3) {
            /* Complex expression using both i and j */
            int t1 = i * 3 + j * 5;
            int t2 = (i ^ j) & 0xFF;
            int t3 = external_func1(t1);
            
            asm volatile("" : : "r"(t1), "r"(t2), "r"(t3));
            
            long long t4 = (long long)t1 * t2 * t3;
            int t5 = external_func2(t2, t3);
            
            asm volatile("" : : "r"(t4), "r"(t5));
            
            long long t6 = t4 + t5 + i + j;
            total += t6;
            
            /* Function call in inner loop increases pressure */
            if (j % 7 == 0) {
                t6 = external_func4(t6);
                total ^= t6;
            }
        }
    }
    
    return total;
}

/* Test 4: Vector operations with 64-bit integers */
long long test_vector_ops(int seed) {
    v4si v1 = {seed, seed + 1, seed + 2, seed + 3};
    v4si v2 = {seed * 2, seed * 3, seed * 4, seed * 5};
    
    /* Multiple vector operations */
    v4si v3 = v1 + v2;
    v4si v4 = v1 * v2;
    v4si v5 = v3 & v4;
    
    asm volatile("" : : "r"(v3), "r"(v4), "r"(v5));
    
    /* Extract and process elements */
    long long ll1 = ((long long)v5[0] << 32) | v5[1];
    long long ll2 = ((long long)v5[2] << 32) | v5[3];
    
    asm volatile("" : : "r"(ll1), "r"(ll2));
    
    long long ll3 = ll1 * ll2;
    ll3 = external_func4(ll3);
    
    long long ll4 = ll2 + ll1 * seed;
    ll4 = external_func4(ll4);
    
    asm volatile("" : : "r"(ll3), "r"(ll4));
    
    return ll3 ^ ll4;
}

/* Test 5: Extreme register pressure with many live values */
int test_extreme_pressure(int seed) {
    /* Create many independent live values */
    int v1 = seed * 1;
    int v2 = seed * 2;
    int v3 = seed * 3;
    int v4 = seed * 4;
    int v5 = seed * 5;
    int v6 = seed * 6;
    int v7 = seed * 7;
    int v8 = seed * 8;
    int v9 = seed * 9;
    int v10 = seed * 10;
    
    /* Force all to registers */
    asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4), "r"(v5));
    asm volatile("" : : "r"(v6), "r"(v7), "r"(v8), "r"(v9), "r"(v10));
    
    /* Complex computation using all values */
    int r1 = v1 + v2 - v3;
    int r2 = v4 * v5 / (v6 + 1);
    int r3 = (v7 & v8) | (v9 ^ v10);
    
    asm volatile("" : : "r"(r1), "r"(r2), "r"(r3));
    
    /* Function calls between computations */
    r1 = external_func1(r1);
    r2 = external_func2(r2, r1);
    r3 = external_func1(r3);
    
    /* More computations */
    int r4 = r1 * r2 + r3;
    int r5 = (r2 << 4) | (r3 >> 4);
    int r6 = r4 ^ r5 ^ 0x12345678;
    
    asm volatile("" : : "r"(r4), "r"(r5), "r"(r6));
    
    /* Final function call */
    r6 = external_func2(r6, r4);
    
    return r6;
}

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : time(NULL);
    volatile int checksum = 0;
    
    printf("Testing early rematerialization coverage...\n");
    
    /* Run all tests to trigger different patterns */
    checksum += test_integer_pressure(seed);
    printf("Test 1 complete\n");
    
    double dresult = test_mixed_types(seed);
    checksum += (int)dresult;
    printf("Test 2 complete\n");
    
    long long llresult = test_nested_loops(seed % 50 + 10);
    checksum += (int)(llresult & 0xFFFFFFFF);
    printf("Test 3 complete\n");
    
    llresult = test_vector_ops(seed);
    checksum += (int)(llresult & 0xFFFFFFFF);
    printf("Test 4 complete\n");
    
    checksum += test_extreme_pressure(seed);
    printf("Test 5 complete\n");
    
    /* Use checksum to prevent optimization */
    asm volatile("" : : "r"(checksum));
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
