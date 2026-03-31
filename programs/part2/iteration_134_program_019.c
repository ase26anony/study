/* test_early_remat.c - Test program for GCC early rematerialization pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External non-inline functions to create register pressure points */
extern int ext_func1(int);
extern double ext_func2(double);
extern long long ext_func3(long long);

/* Vector type to engage different machine modes */
typedef int v4si __attribute__((vector_size(16)));

/* Test 1: High register pressure with mixed integer operations */
int test_mixed_integer_pressure(int seed) {
    volatile int use_result = 0;
    int a = seed * 3 + 1;
    int b = seed / 2 - 5;
    
    /* Long sequence of independent computations */
    int t1 = a * b + seed;
    asm volatile("" : : "r"(t1)); /* Force register allocation */
    
    int t2 = (a & b) | (seed << 2);
    int t3 = t1 ^ t2;
    asm volatile("" : : "r"(t3));
    
    int t4 = t3 * 7 - t2;
    int t5 = t4 / 3 + a;
    
    /* Function call creates register pressure */
    int t6 = ext_func1(t5);
    
    int t7 = t6 * 11 + b;
    int t8 = (t7 << 3) | (t6 >> 2);
    asm volatile("" : : "r"(t8));
    
    int t9 = t8 - t4 * 2;
    int t10 = ext_func1(t9);
    
    int t11 = t10 * 13 + t3;
    int t12 = (t11 & 0xFF) | (t10 << 8);
    asm volatile("" : : "r"(t12));
    
    int t13 = t12 / 5 + t7;
    int t14 = ext_func1(t13);
    
    int t15 = t14 * 17 - t11;
    int t16 = (t15 ^ t14) + t8;
    asm volatile("" : : "r"(t16));
    
    use_result = t16;
    return use_result;
}

/* Test 2: Floating-point operations with different modes */
double test_floating_pressure(double seed) {
    volatile double use_result = 0.0;
    double a = seed * 1.5;
    double b = seed / 3.0;
    
    double f1 = a * b + seed;
    asm volatile("" : : "r"(f1));
    
    double f2 = f1 / 2.0 - b;
    double f3 = ext_func2(f2);
    
    double f4 = f3 * 3.14159 + a;
    asm volatile("" : : "r"(f4));
    
    double f5 = f4 * f4 - f3;
    double f6 = ext_func2(f5);
    
    double f7 = f6 / 1.414 + f2;
    asm volatile("" : : "r"(f7));
    
    double f8 = f7 * f7 + f5;
    double f9 = ext_func2(f8);
    
    use_result = f9;
    return use_result;
}

/* Test 3: Nested loops with complex induction variables */
long long test_nested_loops(int N) {
    volatile long long sum = 0;
    
    for (int i = 0; i < N; i++) {
        /* Non-trivial induction variable derivation */
        for (int j = i * 2; j < N; j += 3) {
            /* Complex expression in inner loop */
            long long val1 = i * 3LL + j;
            long long val2 = (j << 4) | (i & 0xF);
            
            asm volatile("" : : "r"(val1), "r"(val2));
            
            long long val3 = val1 * val2 - i;
            long long val4 = ext_func3(val3);
            
            /* Additional computation to increase pressure */
            long long val5 = val4 / (j + 1) + val1;
            long long val6 = (val5 ^ val2) | val3;
            
            asm volatile("" : : "r"(val5), "r"(val6));
            
            sum += val6;
            
            /* Function call in inner loop */
            if (j % 7 == 0) {
                sum += ext_func3(val5);
            }
        }
    }
    
    return sum;
}

/* Test 4: Vector operations to engage vector modes */
v4si test_vector_operations(int base) {
    v4si v1 = {base, base + 1, base + 2, base + 3};
    v4si v2 = {base + 4, base + 5, base + 6, base + 7};
    
    /* Sequence of vector operations */
    v4si r1 = v1 + v2;
    v4si r2 = v1 * v2;
    asm volatile("" : : "r"(r1), "r"(r2));
    
    v4si r3 = r1 & v2;
    v4si r4 = r2 | v1;
    
    /* Mix with scalar operations */
    int s1 = base * 3;
    int s2 = base / 2;
    asm volatile("" : : "r"(s1), "r"(s2));
    
    v4si r5 = r3 + s1;
    v4si r6 = r4 - s2;
    asm volatile("" : : "r"(r5), "r"(r6));
    
    v4si r7 = r5 * r6;
    v4si r8 = r7 & r1;
    
    return r8;
}

/* Test 5: Mixed data types and complex expressions */
long long test_mixed_types(int seed) {
    volatile long long result = 0;
    
    /* Mix int, float, double, long long */
    int i1 = seed * 3;
    float f1 = seed * 1.5f;
    double d1 = seed * 2.5;
    long long ll1 = seed * 5LL;
    
    asm volatile("" : : "r"(i1), "r"(f1), "r"(d1), "r"(ll1));
    
    /* Cross-type computations */
    double d2 = d1 + f1;
    long long ll2 = ll1 + i1;
    
    int i2 = ext_func1((int)d2);
    double d3 = ext_func2(d2);
    
    asm volatile("" : : "r"(d2), "r"(ll2), "r"(i2), "r"(d3));
    
    float f2 = (float)d3 * f1;
    long long ll3 = ll2 * i2;
    
    /* More complex mixing */
    double d4 = d3 * i2 + f2;
    long long ll4 = ll3 + (long long)d4;
    
    asm volatile("" : : "r"(f2), "r"(ll3), "r"(d4), "r"(ll4));
    
    result = ll4;
    return result;
}

/* Dummy external functions */
int ext_func1(int x) { return x * 2 + 1; }
double ext_func2(double x) { return x * 1.5 - 0.5; }
long long ext_func3(long long x) { return x / 3 + 2; }

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    srand(seed);
    
    printf("Testing early rematerialization coverage...\n");
    
    /* Run all tests to create various register pressure scenarios */
    int r1 = test_mixed_integer_pressure(seed);
    printf("Test 1 result: %d\n", r1);
    
    double r2 = test_floating_pressure(seed * 1.0);
    printf("Test 2 result: %f\n", r2);
    
    long long r3 = test_nested_loops(seed % 50 + 10);
    printf("Test 3 result: %lld\n", r3);
    
    v4si r4 = test_vector_operations(seed);
    printf("Test 4 result vector: {%d, %d, %d, %d}\n", 
           r4[0], r4[1], r4[2], r4[3]);
    
    long long r5 = test_mixed_types(seed);
    printf("Test 5 result: %lld\n", r5);
    
    /* Use results to prevent optimization */
    volatile int final = r1 + (int)r2 + (int)r3 + r4[0] + (int)r5;
    printf("Final checksum: %d\n", final);
    
    return 0;
}
